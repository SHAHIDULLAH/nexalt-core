Nexalt Core integration/staging tree
=====================================

[![Build Status](https://travis-ci.org/nexalt-project/nexalt.svg?branch=master)](https://travis-ci.org/nexalt-project/nexalt)

https://nexalt.org

What is Nexalt?
----------------
Cryptocurrency with the decentralized mass marketing plan
Be a user of Nexalt and get rewards by just referring.

Nexalt is an experimental digital currency that enables instant payments to
anyone, anywhere in the world. Nexalt uses peer-to-peer technology to operate
with no central authority: managing transactions and issuing money are carried
out collectively by the network. Nexalt Core is the name of open source
software which enables the use of this currency.

NEXALT is introducing a unique concept of MLC crypto coins with MLM technology
but not a pyramid design. NEXALT is implementing MLM with a decentralized approach.
Coins are generated through mining and miners become the owners of mined coins,
and a partial reward is given to the sponsors. In this system, bonuses and rewards are
distributed among multiple levels, and every person at every level can work through MLM.
All the workers are the actual owner, and there is no central authority to control the system or system flow.
Thousands of miners are working on the coin generation and become the owners of coins.
The miners use their computing power for mining and as a result, receive a reward in the form of NEXALT coins.

For more information, as well as an immediately useable, binary version of
the Nexalt Core software, see [https://nexalt.org](https://nexalt.org).

License
-------

Nexalt Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/nexalt-project/nexalt/tags) are created
regularly to indicate new official, stable release versions of Nexalt Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

The developer [mailing list](https://groups.google.com/forum/#!forum/nexalt-dev)
should be used to discuss complicated or controversial changes before working
on a patch set.

Developer IRC can be found on Freenode at #nexalt-dev.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/test), written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/test) are installed) with: `test/functional/test_runner.py`

The Travis CI system makes sure that every pull request is built for Windows, Linux, and macOS, and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

We only accept translation fixes that are submitted through [Bitcoin Core's Transifex page](https://www.transifex.com/projects/p/bitcoin/).
Translations are converted to Nexalt periodically.

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.
