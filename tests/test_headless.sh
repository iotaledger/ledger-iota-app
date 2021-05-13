#!/bin/bash
function error {
    echo "error: $1"
    exit 1
}

function usage {
    echo "usage: $0 [-h] [-m|--model (nanos|nanox)]"
    exit 1
}

# default
device="nanos"
realpath="$( dirname $( readlink -f $0 ) )"


FLAGS=""
while (($# > 0))
do
    case "$1" in
    "-h" | "--help")
        usage
        ;;
    "-m" | "--model")
        shift
        device="$1"
        ;;
    *)
        error "unknown parameter"
        ;;
    esac
    shift

done

[[ "$device" != "nanos" && "$device" != "nanox" ]] && {
    error "unknown device"
}
        
[[ "$device" == "nanos" ]] && {
    sdk="2.0"
}

[[ "$device" == "nanox" ]] && {
    sdk="1.2"
}


echo "device $device selected"

cd $realpath

# recompile the app
cd ..
source env_${device}.sh
make clean
SPECULOS=1 make

# start speculos in headless mode in the background
cd ./dev/speculos
python3.8 speculos.py --sdk $sdk --display headless -m ${device}  ../../bin/app.elf &>/dev/null &
speculos_pid=$!
cd -

# wait for speculos to be ready
sleep 10
#netstat -nlp

# now compile test code and run test
cd tests
gcc tests.c -o tests
./tests reference_${device}.bin || error "testing failed for $device"

# kill speculos
kill $speculos_pid

cd -

exit 0



