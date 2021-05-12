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

if [ $(uname) == "Darwin" ]; then
    VOLUME_MOUNT_ARG=""
    xhost + 127.0.0.1
    DISPLAY="host.docker.internal:0"
else
    VOLUME_MOUNT_ARG="-v /tmp/.X11-unix:/tmp/.X11-unix"
    xhost +local:docker
fi
QT_GRAPHICSSYSTEM="native" docker run -p 9999:9999 -it -e DISPLAY=$DISPLAY $VOLUME_MOUNT_ARG build-app bash -c \
"cd /root/git/ledger-iota-app/;"\
"source env_${device}.sh;"\
"make clean;"\
"git pull;"\
"git submodule update --recursive --remote;"\
"SPECULOS=1 make;"\
"cd /root/git/ledger-iota-app/dev/speculos;"\
"python3 speculos.py -m \$( cat ../../../ledger-iota-app/device.txt )  ../../bin/app.elf"
