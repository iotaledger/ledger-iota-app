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
        
echo "device $device selected"


[[ "$device" == "nanox" ]] && {
    error "ledger nanox not (yet) supported"
}


docker run --privileged -v /dev/bus/usb:/dev/bus/usb  -it -v /tmp/.X11-unix:/tmp/.X11-unix ledger_iota bash -c \
"cd /root/git/${device}-secure-sdk;"\
"git pull;"\
"git submodule update --recursive --remote;"\
"cd /root/git/ledger-iota-app/;"\
"source env_${device}.sh;"\
"make clean;"\
"git pull;"\
"DEBUG=1 make load"\
