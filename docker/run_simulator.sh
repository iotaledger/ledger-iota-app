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
sdk=""

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

[[ "$device" != "nanos" && "$device" != "nanox" && "$device" != "nanosplus" ]] && {
    error "unknown device"
}

[[ "$device" == "nanos" ]] && {
    sdk="2.1"
    m="nanos"
}

[[ "$device" == "nanox" ]] && {
    sdk="2.0.2"
    m="nanox"
}

[[ "$device" == "nanosplus" ]] && {
    sdk="1.0"
    m="nanosp"
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

# default Ledger seed
seed="glory promote mansion idle axis finger extra february uncover one trip resource lawn turtle enact monster seven myth punch hobby comfort wild raise skin"

[ -f 'testseed.txt' ] && { seed="$( cat testseed.txt )"; }

rpath="$( dirname $( readlink -f $0 ) )"

QT_GRAPHICSSYSTEM="native" docker run -v $rpath/..:/root/git/app -p 9999:9999 -it -e DISPLAY=$DISPLAY $VOLUME_MOUNT_ARG iotaledger/ledger-build-image:0.0.1 bash -c \
"cd /root/git;"\
"source env_${device}.sh;"\
"cd /root/git/app;"\
"make clean;"\
"SPECULOS=1 make;"\
"cd /opt/ledger/speculos;"\
"SPECULOS_APPNAME=IOTA:0.7.0 python3.8 speculos.py --seed '$seed' --sdk $sdk -m $m /root/git/app/bin/app.elf"
