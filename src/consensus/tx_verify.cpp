// Copyright (c) 2017-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/tx_verify.h>

#include <consensus/consensus.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <consensus/validation.h>

// TODO remove the following dependencies
#include <chain.h>
#include <coins.h>
#include <utilmoneystr.h>

#include <consensus/consensus.h>
#include <consensus/validation.h>
#include <key_io.h>
#include <script/script.h>
#include <script/standard.h>
#include <serialize.h>
#include <streams.h>
#include <univalue.h>
#include <util.h>
#include <utilmoneystr.h>
#include <utilstrencodings.h>

#include <cassert>
#include "leveldb/db.h"
#include <leveldb/c.h>
#include <sstream>
#include <string>
#include <iostream>
#ifdef ENABLE_WALLET
#include <../wallet/rpcwallet.h>
#endif
#include <../wallet/coincontrol.h>
#include <../wallet/feebumper.h>
#include <../wallet/rpcwallet.h>
#include <../wallet/wallet.h>
#include <../wallet/walletdb.h>
#include <../wallet/walletutil.h>
#include <future>
#include <stdint.h>
#include <univalue.h>
#include <core_io.h>
#include <consensus/consensus.h>
#include <consensus/validation.h>
#include <key_io.h>
#include <script/script.h>
#include <script/standard.h>
#include <serialize.h>
#include <streams.h>
#include <univalue.h>
#include <util.h>
#include <utilmoneystr.h>
#include <utilstrencodings.h>

using namespace std;

vector <string> explode_raw_tx_verify(const string &delimiter, const string &explodeme);

vector <string> explode_raw_tx_verify(const string &delimiter, const string &str) {
    vector <string> arr;

    int strleng = str.length();
    int delleng = delimiter.length();
    if (delleng == 0)
        return arr;//no change

    int i = 0;
    int k = 0;
    while (i < strleng) {
        int j = 0;
        while (i + j < strleng && j < delleng && str[i + j] == delimiter[j])
            j++;
        if (j == delleng)//found delimiter
        {
            arr.push_back(str.substr(k, i - k));
            i += delleng;
            k = i;
        } else {
            i++;
        }
    }
    arr.push_back(str.substr(k, i - k));
    return arr;
}

int hex_value_tx_verify(char hex_digit) {
    switch (hex_digit) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return hex_digit - '0';

        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            return hex_digit - 'A' + 10;

        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            return hex_digit - 'a' + 10;
    }
    throw std::invalid_argument("invalid hex digit");
}

string stringToHex_tx_verify(string str) {
    static const char hex_digits[] = "0123456789abcdef";
    std::string output;
    output.reserve(str.length() * 2);
    for (unsigned char c : str) {
        output.push_back(hex_digits[c >> 4]);
        output.push_back(hex_digits[c & 15]);
    }
    return output;
}

string hexToString_tx_verify(string str) {
    const auto len = str.length();
    if (len & 1) throw std::invalid_argument("odd length");
    std::string output_string;
    output_string.reserve(len / 2);
    for (auto it = str.begin(); it != str.end();) {
        int hi = hex_value_tx_verify(*it++);
        int lo = hex_value_tx_verify(*it++);
        output_string.push_back(hi << 4 | lo);
    }
    return output_string;
}


bool IsFinalTx(const CTransaction &tx, int nBlockHeight, int64_t nBlockTime)
{
    if (tx.nLockTime == 0)
        return true;
    if ((int64_t)tx.nLockTime < ((int64_t)tx.nLockTime < LOCKTIME_THRESHOLD ? (int64_t)nBlockHeight : nBlockTime))
        return true;
    for (const auto& txin : tx.vin) {
        if (!(txin.nSequence == CTxIn::SEQUENCE_FINAL))
            return false;
    }
    return true;
}

std::pair<int, int64_t> CalculateSequenceLocks(const CTransaction &tx, int flags, std::vector<int>* prevHeights, const CBlockIndex& block)
{
    assert(prevHeights->size() == tx.vin.size());

    // Will be set to the equivalent height- and time-based nLockTime
    // values that would be necessary to satisfy all relative lock-
    // time constraints given our view of block chain history.
    // The semantics of nLockTime are the last invalid height/time, so
    // use -1 to have the effect of any height or time being valid.
    int nMinHeight = -1;
    int64_t nMinTime = -1;

    // tx.nVersion is signed integer so requires cast to unsigned otherwise
    // we would be doing a signed comparison and half the range of nVersion
    // wouldn't support BIP 68.
    bool fEnforceBIP68 = static_cast<uint32_t>(tx.nVersion) >= 2
                         && flags & LOCKTIME_VERIFY_SEQUENCE;

    // Do not enforce sequence numbers as a relative lock time
    // unless we have been instructed to
    if (!fEnforceBIP68) {
        return std::make_pair(nMinHeight, nMinTime);
    }

    for (size_t txinIndex = 0; txinIndex < tx.vin.size(); txinIndex++) {
        const CTxIn& txin = tx.vin[txinIndex];

        // Sequence numbers with the most significant bit set are not
        // treated as relative lock-times, nor are they given any
        // consensus-enforced meaning at this point.
        if (txin.nSequence & CTxIn::SEQUENCE_LOCKTIME_DISABLE_FLAG) {
            // The height of this input is not relevant for sequence locks
            (*prevHeights)[txinIndex] = 0;
            continue;
        }

        int nCoinHeight = (*prevHeights)[txinIndex];

        if (txin.nSequence & CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG) {
            int64_t nCoinTime = block.GetAncestor(std::max(nCoinHeight-1, 0))->GetMedianTimePast();
            // NOTE: Subtract 1 to maintain nLockTime semantics
            // BIP 68 relative lock times have the semantics of calculating
            // the first block or time at which the transaction would be
            // valid. When calculating the effective block time or height
            // for the entire transaction, we switch to using the
            // semantics of nLockTime which is the last invalid block
            // time or height.  Thus we subtract 1 from the calculated
            // time or height.

            // Time-based relative lock-times are measured from the
            // smallest allowed timestamp of the block containing the
            // txout being spent, which is the median time past of the
            // block prior.
            nMinTime = std::max(nMinTime, nCoinTime + (int64_t)((txin.nSequence & CTxIn::SEQUENCE_LOCKTIME_MASK) << CTxIn::SEQUENCE_LOCKTIME_GRANULARITY) - 1);
        } else {
            nMinHeight = std::max(nMinHeight, nCoinHeight + (int)(txin.nSequence & CTxIn::SEQUENCE_LOCKTIME_MASK) - 1);
        }
    }

    return std::make_pair(nMinHeight, nMinTime);
}

bool EvaluateSequenceLocks(const CBlockIndex& block, std::pair<int, int64_t> lockPair)
{
    assert(block.pprev);
    int64_t nBlockTime = block.pprev->GetMedianTimePast();
    if (lockPair.first >= block.nHeight || lockPair.second >= nBlockTime)
        return false;

    return true;
}

bool SequenceLocks(const CTransaction &tx, int flags, std::vector<int>* prevHeights, const CBlockIndex& block)
{
    return EvaluateSequenceLocks(block, CalculateSequenceLocks(tx, flags, prevHeights, block));
}

unsigned int GetLegacySigOpCount(const CTransaction& tx)
{
    unsigned int nSigOps = 0;
    for (const auto& txin : tx.vin)
    {
        nSigOps += txin.scriptSig.GetSigOpCount(false);
    }
    for (const auto& txout : tx.vout)
    {
        nSigOps += txout.scriptPubKey.GetSigOpCount(false);
    }
    return nSigOps;
}

unsigned int GetP2SHSigOpCount(const CTransaction& tx, const CCoinsViewCache& inputs)
{
    if (tx.IsCoinBase())
        return 0;

    unsigned int nSigOps = 0;
    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
        const Coin& coin = inputs.AccessCoin(tx.vin[i].prevout);
        assert(!coin.IsSpent());
        const CTxOut &prevout = coin.out;
        if (prevout.scriptPubKey.IsPayToScriptHash())
            nSigOps += prevout.scriptPubKey.GetSigOpCount(tx.vin[i].scriptSig);
    }
    return nSigOps;
}

int64_t GetTransactionSigOpCost(const CTransaction& tx, const CCoinsViewCache& inputs, int flags)
{
    int64_t nSigOps = GetLegacySigOpCount(tx) * WITNESS_SCALE_FACTOR;

    if (tx.IsCoinBase())
        return nSigOps;

    if (flags & SCRIPT_VERIFY_P2SH) {
        nSigOps += GetP2SHSigOpCount(tx, inputs) * WITNESS_SCALE_FACTOR;
    }

    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
        const Coin& coin = inputs.AccessCoin(tx.vin[i].prevout);
        assert(!coin.IsSpent());
        const CTxOut &prevout = coin.out;
        nSigOps += CountWitnessSigOps(tx.vin[i].scriptSig, prevout.scriptPubKey, &tx.vin[i].scriptWitness, flags);
    }
    return nSigOps;
}

bool CheckTransaction(const CTransaction& tx, CValidationState &state, bool fCheckDuplicateInputs)
{
    // Basic checks that don't depend on any context
    if (tx.vin.empty())
        return state.DoS(10, false, REJECT_INVALID, "bad-txns-vin-empty");
    if (tx.vout.empty())
        return state.DoS(10, false, REJECT_INVALID, "bad-txns-vout-empty");
    // Size limits (this doesn't take the witness into account, as that hasn't been checked for malleability)
    if (::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS) * WITNESS_SCALE_FACTOR > MAX_BLOCK_WEIGHT)
        return state.DoS(100, false, REJECT_INVALID, "bad-txns-oversize");

    // Check for negative or overflow output values
    CAmount nValueOut = 0;
    for (const auto& txout : tx.vout)
    {
        if (txout.nValue < 0)
            return state.DoS(100, false, REJECT_INVALID, "bad-txns-vout-negative");
        if (txout.nValue > MAX_MONEY)
            return state.DoS(100, false, REJECT_INVALID, "bad-txns-vout-toolarge");
        nValueOut += txout.nValue;
        if (!MoneyRange(nValueOut))
            return state.DoS(100, false, REJECT_INVALID, "bad-txns-txouttotal-toolarge");
    }

    // Check for duplicate inputs - note that this check is slow so we skip it in CheckBlock
    if (fCheckDuplicateInputs) {
        std::set<COutPoint> vInOutPoints;
        for (const auto& txin : tx.vin)
        {
            if (!vInOutPoints.insert(txin.prevout).second)
                return state.DoS(100, false, REJECT_INVALID, "bad-txns-inputs-duplicate");
        }
    }

    if (tx.IsCoinBase())
    {
        if (tx.vin[0].scriptSig.size() < 2 || tx.vin[0].scriptSig.size() > 100)
            return state.DoS(100, false, REJECT_INVALID, "bad-cb-length");
    }
    else
    {
        for (const auto& txin : tx.vin)
            if (txin.prevout.IsNull())
                return state.DoS(10, false, REJECT_INVALID, "bad-txns-prevout-null");
    }

    return true;
}

bool CheckTransactionToGetData(const CTransaction &tx, CValidationState &state, int nHeight , double mlcDistribution ,
                               bool fCheckDuplicateInputs) {

    double subsidy = (mlcDistribution * 33.34 / 100 ) / COIN;
    double newsubsidy = (subsidy * 10) /100 ;

    // Basic checks that don't depend on any context
    if (tx.vin.empty())
        return state.DoS(10, false, REJECT_INVALID, "bad-txns-vin-empty");
    if (tx.vout.empty())
        return state.DoS(10, false, REJECT_INVALID, "bad-txns-vout-empty");
    // Size limits (this doesn't take the witness into account, as that hasn't been checked for malleability)
    if (::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS) *
        WITNESS_SCALE_FACTOR > MAX_BLOCK_WEIGHT)
        return state.DoS(100, false, REJECT_INVALID, "bad-txns-oversize");

    // Check for negative or overflow output values
    CAmount nValueOut = 0;
    for (const auto &txout : tx.vout) {
        if (txout.nValue < 0)
            return state.DoS(100, false, REJECT_INVALID, "bad-txns-vout-negative");
        if (txout.nValue > MAX_MONEY)
            return state.DoS(100, false, REJECT_INVALID, "bad-txns-vout-toolarge");
        nValueOut += txout.nValue;
        if (!MoneyRange(nValueOut))
            return state.DoS(100, false, REJECT_INVALID, "bad-txns-txouttotal-toolarge");
    }
    // Check for duplicate inputs - note that this check is slow so we skip it in CheckBlock
    if (fCheckDuplicateInputs) {
        std::set <COutPoint> vInOutPoints;
        for (const auto &txin : tx.vin) {
            if (!vInOutPoints.insert(txin.prevout).second)
                return state.DoS(100, false, REJECT_INVALID, "bad-txns-inputs-duplicate");
        }
    }

    string hexconverted = "";
    std::string std_data_dir = GetDataDir().string();
    leveldb::Status status;
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;

    if (tx.IsCoinBase()) {
        if (tx.vin[0].scriptSig.size() < 2 || tx.vin[0].scriptSig.size() > 100) {
            return state.DoS(100, false, REJECT_INVALID, "bad-cb-length");
        }
        if (nHeight > 110556) {
            int i = 0;
            string secondvalue ="";
            for (const auto &txout : tx.vout) {
                std::string valueToCheck = "";
                std::string SecondvalueToCheck = "";
                std::string addresscoinbase = "";

                const CScript &scriptPubKey = txout.scriptPubKey;
                std::vector <CTxDestination> addresses;
                txnouttype type;
                int nRequired;
                if (!ExtractDestinations(scriptPubKey, type, addresses, nRequired)) {
                    //address not found
                }
                for (const CTxDestination &addr : addresses) {
                    //address=EncodeDestination(addr) value=txout.nValu
                    if(i == 0){
                        //
                    }else{
                        if(txout.nValue < newsubsidy ){
                            //reject here because reward is less
                            //return state.DoS(100, false, REJECT_INVALID, "bad-cb-length");
                            return state.DoS(100, false, REJECT_INVALID, "bad-cb-missing", false, "bad Reward Amount found in coinBase.");
                        }
                        addresscoinbase = EncodeDestination(addr);
                        status = leveldb::DB::Open(options, std_data_dir + "/mlc", &db);
                        if (status.ok()) status = db->Get(leveldb::ReadOptions(), addresscoinbase, &valueToCheck);
                        delete db;
                        if (valueToCheck == "") {
                            //std::cout<<"first is not sponsor\n";
                            //this is not the sponser so reject block
                            return state.DoS(100, false, REJECT_INVALID, "bad-cb-missing", false, "bad MLC found in coinBase.");
                        } else {
                            if(i==1){
                                //cout<<"means that i=1 and secondvalue = valueToCheck\n";
                                secondvalue = valueToCheck;
                            }else{
                                status = leveldb::DB::Open(options, std_data_dir + "/mlc", &db);
                                if (status.ok()) status = db->Get(leveldb::ReadOptions(), secondvalue, &SecondvalueToCheck);
                                delete db;
                                secondvalue = valueToCheck;
                                if(valueToCheck == SecondvalueToCheck){
                                    secondvalue = valueToCheck;
                                }else{
                                    return state.DoS(100, false, REJECT_INVALID, "bad-cb-missing", false, "bad MLC found that not matches in coinBase.");
                                }
                            }
                        }
                    }
                }
                i++;
            }
        }
    } else {
        for (const auto &txout : tx.vout) {
            std::string asmstring = ScriptToAsmStr(txout.scriptPubKey);
            std::vector <std::string> no_keys = explode_raw_tx_verify(" ", asmstring);
            if (no_keys[0] == "OP_RETURN") {
                hexconverted = hexToString_tx_verify(no_keys[1]);
                std::string mlc = hexconverted.substr(0, 3);
                std::string KoT = hexconverted.substr(3, 1);
                if (mlc == "MLC") {
                    if (KoT == "K") {
                        std::string key = hexconverted.substr(4, 34);
                        std::string value = hexconverted.substr(38, 34);

                        for (const std::shared_ptr<CWallet>& pwallet : GetWallets()) {
                            std::string wallet_name = pwallet->GetName();
                            CTxDestination dest = DecodeDestination(key);
                            std::shared_ptr <CWallet> wallet = GetWallet(wallet_name);
                            if(wallet != 0){
                                isminetype mine = IsMine(*wallet, dest);
                                if (bool(mine & ISMINE_SPENDABLE) == 1){
                                    if(wallet_name == ""){
                                        //default wallet
                                        leveldb::Status status_my;
                                        leveldb::DB *db_my;
                                        leveldb::Options options_my;
                                        options_my.create_if_missing = true;
                                        std::string StringKey = "StringKey";
                                        std::string StringKeyToShow = "StringKeyToShow";
                                        status_my = leveldb::DB::Open(options_my, std_data_dir + "/myKey", &db_my);
                                        if (status_my.ok()) status_my = db_my->Put(leveldb::WriteOptions(), StringKey, key);
                                        if (status_my.ok()) status_my = db_my->Put(leveldb::WriteOptions(), StringKeyToShow, tx.GetHash().ToString());
                                        delete db_my;
                                    }else{
                                        leveldb::Status status_my;
                                        leveldb::DB *db_my;
                                        leveldb::Options options_my;
                                        options_my.create_if_missing = true;
                                        std::string StringKey = wallet_name;
                                        std::string StringKeyToShow = wallet_name +"_show";
                                        status_my = leveldb::DB::Open(options_my, std_data_dir + "/myKey", &db_my);
                                        if (status_my.ok()) status_my = db_my->Put(leveldb::WriteOptions(), StringKey, key);
                                        if (status_my.ok()) status_my = db_my->Put(leveldb::WriteOptions(), StringKeyToShow, tx.GetHash().ToString());
                                        delete db_my;
                                    }
                                }
                            }
                        }
                        std::string valueToCheck = "";
                        status = leveldb::DB::Open(options, std_data_dir + "/mlc", &db);
                        if (status.ok()) status = db->Get(leveldb::ReadOptions(), key, &valueToCheck);
                        delete db;
                        if (valueToCheck == "") {
                            if (nHeight > 110556) {
                                std::string MlcKeyCheck = "";
                                status = leveldb::DB::Open(options, std_data_dir + "/mlc", &db);
                                if (status.ok()) status = db->Get(leveldb::ReadOptions(), value, &MlcKeyCheck);
                                delete db;
                                if(MlcKeyCheck == ""){
                                    //std::cout << "Not Saving it because this is wrong chain key= " << key << "\n";
                                    //std::cout << "Not Saving it because this is wrong chain value= " << value << "\n";
                                }else{
                                    status = leveldb::DB::Open(options, std_data_dir + "/mlc", &db);
                                    if (status.ok()) status = db->Put(leveldb::WriteOptions(), key, value);
                                    delete db;
                                }
                            }else{
                                status = leveldb::DB::Open(options, std_data_dir + "/mlc", &db);
                                if (status.ok()) status = db->Put(leveldb::WriteOptions(), key, value);
                                delete db;
                            }
                        }
                    }
                }
            }
        }
        for (const auto &txin : tx.vin)
            if (txin.prevout.IsNull())
                return state.DoS(10, false, REJECT_INVALID, "bad-txns-prevout-null");
    }
    return true;
}

bool Consensus::CheckTxInputs(const CTransaction& tx, CValidationState& state, const CCoinsViewCache& inputs, int nSpendHeight, CAmount& txfee)
{
    // are the actual inputs available?
    if (!inputs.HaveInputs(tx)) {
        return state.DoS(100, false, REJECT_INVALID, "bad-txns-inputs-missingorspent", false,
                         strprintf("%s: inputs missing/spent", __func__));
    }

    CAmount nValueIn = 0;
    for (unsigned int i = 0; i < tx.vin.size(); ++i) {
        const COutPoint &prevout = tx.vin[i].prevout;
        const Coin& coin = inputs.AccessCoin(prevout);
        assert(!coin.IsSpent());

        // If prev is coinbase, check that it's matured
        if (coin.IsCoinBase() && nSpendHeight - coin.nHeight < COINBASE_MATURITY) {
            return state.Invalid(false,
                                 REJECT_INVALID, "bad-txns-premature-spend-of-coinbase",
                                 strprintf("tried to spend coinbase at depth %d", nSpendHeight - coin.nHeight));
        }

        // Check for negative or overflow input values
        nValueIn += coin.out.nValue;
        if (!MoneyRange(coin.out.nValue) || !MoneyRange(nValueIn)) {
            return state.DoS(100, false, REJECT_INVALID, "bad-txns-inputvalues-outofrange");
        }
    }

    const CAmount value_out = tx.GetValueOut();
    if (nValueIn < value_out) {
        return state.DoS(100, false, REJECT_INVALID, "bad-txns-in-belowout", false,
                         strprintf("value in (%s) < value out (%s)", FormatMoney(nValueIn), FormatMoney(value_out)));
    }

    // Tally transaction fees
    const CAmount txfee_aux = nValueIn - value_out;
    if (!MoneyRange(txfee_aux)) {
        return state.DoS(100, false, REJECT_INVALID, "bad-txns-fee-outofrange");
    }

    txfee = txfee_aux;
    return true;
}
