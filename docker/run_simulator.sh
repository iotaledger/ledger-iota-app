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

xhost +local:docker
QT_GRAPHICSSYSTEM="native" docker run -p 9999:9999 -it -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix ledger_iota bash -c \
"cd /root/git/${device}-secure-sdk;"\
"git pull;"\
"cd /root/git/ledger-iota-app/;"\
"source env_${device}.sh;"\
"make clean;"\
"git pull;"\
"git submodule update --recursive --remote;"\
"SPECULOS=1 make;"\
"cd /root/git/speculos;"\
"python3 speculos.py -m \$( cat ../ledger-iota-app/device.txt )  ../ledger-iota-app/bin/app.elf"
