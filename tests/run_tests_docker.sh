#!/bin/bash
echo
echo "testing NANO S"
docker run -it test-app bash -c "cd /root/git/ledger-iota-app/tests; chmod a+x test_headless.sh; ./test_headless.sh -m nanos" || exit 1
echo
echo "testing NANO X"
docker run -it test-app bash -c "cd /root/git/ledger-iota-app/tests; chmod a+x test_headless.sh; ./test_headless.sh -m nanox" || exit 1
