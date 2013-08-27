#!/bin/bash

trap 'kill $(jobs -p)' EXIT
./TestServer 2>/dev/null 1>/dev/null &
wstest -m fuzzingclient -s fuzzingclient.json

