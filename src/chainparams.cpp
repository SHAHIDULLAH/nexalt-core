// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <consensus/merkle.h>
#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>
#include <assert.h>
#include <chainparamsseeds.h>

static CBlock
CreateGenesisBlock(const char *pszTimestamp, const CScript &genesisOutputScript, uint32_t nTime, uint32_t nNonce,
                   uint32_t nBits, int32_t nVersion, const CAmount &genesisReward) {
        CMutableTransaction txNew;
        txNew.nVersion = 1;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4)
                                           << std::vector<unsigned char>((const unsigned char *) pszTimestamp,
                                                                         (const unsigned char *) pszTimestamp +
                                                                         strlen(pszTimestamp));
        txNew.vout[0].nValue = genesisReward;
        txNew.vout[0].scriptPubKey = genesisOutputScript;

        CBlock genesis;
        genesis.nTime = nTime;
        genesis.nBits = nBits;
        genesis.nNonce = nNonce;
        genesis.nVersion = nVersion;
        genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
        genesis.hashPrevBlock.SetNull();
        genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
        return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock
CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount &genesisReward) {
        const char *pszTimestamp = "BBC 02/02/2020 UK begins new chapter outside European Union";
        const CScript genesisOutputScript = CScript() << ParseHex(
                "040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9")
                                                      << OP_CHECKSIG;
        return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout) {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
            strNetworkID = "main";
            consensus.nSubsidyHalvingInterval = 840000;
            consensus.BIP16Height = 218579; // 87afb798a3ad9378fcd56123c81fb31cfd9a8df4719b9774d71730c16315a092 - October 1, 2012
            consensus.BIP34Height = 710000;
            consensus.BIP34Hash = uint256S("fa09d204a83a768ed5a7c8d441fa62f2043abf420cff1226c7b4329aeb9d51cf");
            consensus.BIP65Height = 918684; // bab3041e8977e0dc3eeff63fe707b92bde1dd449d8efafb248c27c8264cc311a
            consensus.BIP66Height = 811879; // 7aceee012833fa8952f8835d8b1b3ae233cd6ab08fdb27a771d2bd7bdc491894
            consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days
            consensus.nPowTargetSpacing = 2.5 * 60;
            consensus.fPowAllowMinDifficultyBlocks = false;
            consensus.fPowNoRetargeting = false;
            consensus.nRuleChangeActivationThreshold = 6048; // 75% of 8064
            consensus.nMinerConfirmationWindow = 8064; // nPowTargetTimespan / nPowTargetSpacing * 4
            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

            // Deployment of BIP68, BIP112, and BIP113.
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1485561600; // January 28, 2017
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1517356801; // January 31st, 2018

            // Deployment of SegWit (BIP141, BIP143, and BIP147)
            consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
            consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1485561600; // January 28, 2017
            consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 0; // January 31st, 2018

            // The best chain should have at least this much work.
            consensus.nMinimumChainWork = uint256S(
                    "0x0000000000000000000000000000000000000000000000000000000000000000");

            // By default assume that the signatures in ancestors of this block are valid.
            consensus.defaultAssumeValid = uint256S(
                    "0xa601455787cb65ffc325dda4751a99cf01d1567799ec4b04f45bb05f9ef0cbde"); //1503191

            /**
             * The message start string is designed to be unlikely to occur in normal data.
             * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
             * a large 32-bit integer with any alignment.
             */
            pchMessageStart[0] = 0xd0;
            pchMessageStart[1] = 0xe1;
            pchMessageStart[2] = 0xf5;
            pchMessageStart[3] = 0xec;
            nDefaultPort = 55786;
            nPruneAfterHeight = 100000;

            genesis = CreateGenesisBlock(1580601602, 2086557703, 0x1e0ffff0, 1, 50 * COIN);
            consensus.hashGenesisBlock = genesis.GetHash();
            assert(consensus.hashGenesisBlock ==
                   uint256S("0xd4f99f684bca3f717ce78f6b75ef4d77a9228a91eb472fc09d647e9007195566"));
            assert(genesis.hashMerkleRoot ==
                   uint256S("0x9a9d43fe8ca34a211028618a53782000ec0f382bededc0d5e283667b9d08f043"));

            // Note that of those which support the service bits prefix, most only support a subset of
            // possible options.
            // This is fine at runtime as we'll fall back to using them as a oneshot if they don't support the
            // service bits we want, but we should get them updated to support all service bits wanted by any
            // release ASAP to avoid it where possible.

            vSeeds.emplace_back("dnsseed.nexalttools.com");
            vSeeds.emplace_back("dnsseed2.nexalttools.com");
            vSeeds.emplace_back("server1.nexalttools.com");
            //vSeeds.emplace_back("dnsseeds.nexalt.live");
            //vSeeds.emplace_back("nexalt1.itempire.info");
        /*vSeeds.emplace_back("dnsseed.thrasher.io");
        vSeeds.emplace_back("dnsseed.nexalttools.com");
        vSeeds.emplace_back("dnsseed.nexaltpool.org");
        vSeeds.emplace_back("dnsseed.koin-project.com");*/

            base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 74);
            base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 5);
            base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1, 50);
            base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 74);
            base58Prefixes[EXT_PUBLIC_KEY] = {0xff, 0x88, 0xB2, 0x1E};
            base58Prefixes[EXT_SECRET_KEY] = {0xff, 0x88, 0xAD, 0xE4};

            bech32_hrp = "xlt";

            vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

            fDefaultConsistencyChecks = false;
            fRequireStandard = true;
            fMineBlocksOnDemand = false;

        checkpointData = {
                {
                        {0, uint256S("0xd4f99f684bca3f717ce78f6b75ef4d77a9228a91eb472fc09d647e9007195566")},
                        {20000, uint256S("0x38c56869937721d0ce6bd17a64051c25bebd6f6d5d9bea50905899b24bdacd92")},
                        {25000, uint256S("0x886c72b1020b0c9fef8a990226bf4da30d3c64da107ce61de03426ec9d129f55")},
                        {30000, uint256S("0x34de22566e0680288cbbbc445a5d9d489bbb2d58b84ba5f46929150a95251b82")},
                        {35000, uint256S("0x62a24ea1dd1c29b6b4e52d81a0978331ee19fdf7c68a44da5c666ced5d02e449")},
                        {37069, uint256S("0x155d3a3b33833223cf3fd4d607ee48b6f7a7a4d8b5f8930ab10c16587778539d")},
                }
        };

        chainTxData = ChainTxData{
                // Data from rpc: getchaintxstats 4096 a601455787cb65ffc325dda4751a99cf01d1567799ec4b04f45bb05f9ef0cbde
                /* nTime    */ 1585290088,
                /* nTxCount */ 43594,
                /* dTxRate  */ 0.011
        };
            /* disable fallback fee on mainnet */
            m_fallback_fee_enabled = false;
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
            strNetworkID = "test";
            consensus.nSubsidyHalvingInterval = 840000;
            consensus.BIP16Height = 0; // always enforce P2SH BIP16 on testnet
            consensus.BIP34Height = 76;
            consensus.BIP34Hash = uint256S("8075c771ed8b495ffd943980a95f702ab34fce3c8c54e379548bda33cc8c0573");
            consensus.BIP65Height = 76; // 8075c771ed8b495ffd943980a95f702ab34fce3c8c54e379548bda33cc8c0573
            consensus.BIP66Height = 76; // 8075c771ed8b495ffd943980a95f702ab34fce3c8c54e379548bda33cc8c0573
            consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days
            consensus.nPowTargetSpacing = 2.5 * 60;
            consensus.fPowAllowMinDifficultyBlocks = true;
            consensus.fPowNoRetargeting = false;
            consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
            consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

            // Deployment of BIP68, BIP112, and BIP113.
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1483228800; // January 1, 2017
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1517356801; // January 31st, 2018

            // Deployment of SegWit (BIP141, BIP143, and BIP147)
            consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
            consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1483228800; // January 1, 2017
            consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1517356801; // January 31st, 2018

            // The best chain should have at least this much work.
            consensus.nMinimumChainWork = uint256S(
                    "0x0000000000000000000000000000000000000000000000000000000000000000");

            // By default assume that the signatures in ancestors of this block are valid.
            consensus.defaultAssumeValid = uint256S(
                    "0x438b7e1c86f58b4e62e8cf2d0f9f256e3dddaebc5d1cc568e633a38e0db6c025"); //787085

            pchMessageStart[0] = 0xd0;
            pchMessageStart[1] = 0xe1;
            pchMessageStart[2] = 0xf5;
            pchMessageStart[3] = 0xec;
            nDefaultPort = 19662;
            nPruneAfterHeight = 1000;


        genesis = CreateGenesisBlock(1580601602, 2086557703, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock ==
               uint256S("0xd4f99f684bca3f717ce78f6b75ef4d77a9228a91eb472fc09d647e9007195566"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0x9a9d43fe8ca34a211028618a53782000ec0f382bededc0d5e283667b9d08f043"));

            vFixedSeeds.clear();
            vSeeds.clear();
            // nodes with support for servicebits filtering should be at the top
            /*vSeeds.emplace_back("testnet-seed.nexalttools.com");
            vSeeds.emplace_back("seed-b.nexalt.loshan.co.uk");
            vSeeds.emplace_back("dnsseed-testnet.thrasher.io");*/

            base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 54);
            base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
            base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1, 58);
            base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 54);
            base58Prefixes[EXT_PUBLIC_KEY] = {0xff, 0x35, 0x87, 0xCF};
            base58Prefixes[EXT_SECRET_KEY] = {0xff, 0x35, 0x83, 0x94};

            bech32_hrp = "txlt";

            vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

            fDefaultConsistencyChecks = false;
            fRequireStandard = false;
            fMineBlocksOnDemand = false;

            checkpointData = (CCheckpointData) {
                    {
                            {0, uint256S("19728567f5c58ab90c94c900b2dab92a16b7a1dc945b71d40d55ccbb384f5172")},
                    }
            };

            chainTxData = ChainTxData{
                    // Data from rpc: getchaintxstats 4096 438b7e1c86f58b4e62e8cf2d0f9f256e3dddaebc5d1cc568e633a38e0db6c025
                    /* nTime    */ 1579031503,
                    /* nTxCount */ 0,
                    /* dTxRate  */ 0
            };

            /* enable fallback fee on testnet */
            m_fallback_fee_enabled = true;
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
            strNetworkID = "regtest";
            consensus.nSubsidyHalvingInterval = 150;
            consensus.BIP16Height = 0; // always enforce P2SH BIP16 on regtest
            consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
            consensus.BIP34Hash = uint256();
            consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
            consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
            consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60; // two weeks
            consensus.nPowTargetSpacing = 2.5 * 60;
            consensus.fPowAllowMinDifficultyBlocks = true;
            consensus.fPowNoRetargeting = true;
            consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
            consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
            consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
            consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
            consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

            // The best chain should have at least this much work.
            consensus.nMinimumChainWork = uint256S("0x00");

            // By default assume that the signatures in ancestors of this block are valid.
            consensus.defaultAssumeValid = uint256S("0x00");

            pchMessageStart[0] = 0xfa;
            pchMessageStart[1] = 0xbf;
            pchMessageStart[2] = 0xb5;
            pchMessageStart[3] = 0xda;
            nDefaultPort = 19444;
            nPruneAfterHeight = 1000;

            genesis = CreateGenesisBlock(1296688602, 0, 0x207fffff, 1, 50 * COIN);
            consensus.hashGenesisBlock = genesis.GetHash();
            assert(consensus.hashGenesisBlock ==
                   uint256S("0x530827f38f93b43ed12af0b3ad25a288dc02ed74d6d7857862df51fc56c416f9"));
            assert(genesis.hashMerkleRoot ==
                   uint256S("0x97ddfbbae6be97fd6cdf3e7ca13232a3afff2353e29badfab7f73011edd4ced9"));

            vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
            vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

            fDefaultConsistencyChecks = true;
            fRequireStandard = false;
            fMineBlocksOnDemand = true;

            checkpointData = {
                    {
                            {0, uint256S("530827f38f93b43ed12af0b3ad25a288dc02ed74d6d7857862df51fc56c416f9")},
                    }
            };

            chainTxData = ChainTxData{
                    0,
                    0,
                    0
            };

            base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 111);
            base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
            base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1, 58);
            base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);
            base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
            base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

            bech32_hrp = "rxlt";

            /* enable fallback fee on regtest */
            m_fallback_fee_enabled = true;
    }
};

static std::unique_ptr <CChainParams> globalChainParams;

const CChainParams &Params() {
        assert(globalChainParams);
        return *globalChainParams;
}

std::unique_ptr <CChainParams> CreateChainParams(const std::string &chain) {
        if (chain == CBaseChainParams::MAIN)
                return std::unique_ptr<CChainParams>(new CMainParams());
        else if (chain == CBaseChainParams::TESTNET)
                return std::unique_ptr<CChainParams>(new CTestNetParams());
        else if (chain == CBaseChainParams::REGTEST)
                return std::unique_ptr<CChainParams>(new CRegTestParams());
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string &network) {
        SelectBaseParams(network);
        globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout) {
        globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
