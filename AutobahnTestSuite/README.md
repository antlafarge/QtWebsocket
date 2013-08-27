Prerequisites
=============

AutobahnTestSuite (see http://autobahn.ws/testsuite/installation)

Running Tests
=============

`cd AutobahnTestSuite && make test`
This compiles the test server and runs `autobahntest.sh`.
Check `reports/server/index.html` for the test suite report.

Configuring Test Suite
======================

The test suite can be configured be editing `fuzzingclient.json`. By default the
UTF-8 and performance tests are skipped. All other tests should pass.
For more on the configuration options refer to AutobahnTestSuite documentation.
