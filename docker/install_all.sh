#!/bin/bash

	

export DEBIAN_FRONTEND='noninteractive'

function error {
    echo "error: $1"
    exit 1
}

[[ "$1" != "DOCKER" ]] && {
    error "don't run the script outside of the docker build!"
}

[[ "$DEVICE" != "nanos" && "$DEVICE" != "nanox" ]] && {
    error "unknown device"
}

mkdir git
mkdir Downloads

apt update

apt -y install vim curl git wget

apt -y install python3 python3-pip libusb-dev libusb-1.0-0-dev libudev-dev cmake  qemu-user-static python3-pyqt5 python3-construct python3-jsonschema python3-mnemonic python3-pyelftools gcc-arm-linux-gnueabihf libc6-dev-armhf-cross gdb-multiarch pkg-config  libssl-dev protobuf-compiler || error "installing packages"

cd Downloads

echo
echo "downloading clang ..."
wget 'https://releases.llvm.org/7.0.0/clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz' -q -O clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz
echo "downloading gcc ..."
wget 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/5_3-2016q1/gccarmnoneeabi532016q120160330linuxtar.bz2' -q -O gccarmnoneeabi532016q120160330linuxtar.bz2

unxz clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz
tar xvf clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04.tar

tar xjvf gccarmnoneeabi532016q120160330linuxtar.bz2

cd -

cd git

git clone --recurse-submodules https://gitlab.com/ledger-iota-chrysalis/ledger-iota-app && \
git clone https://github.com/LedgerHQ/speculos && \
git clone https://github.com/LedgerHQ/blue-loader-python || error "cloning repositories"





# build blue loader python
pip3 install Pillow || error "Pillow not found"

cd blue-loader-python
python3 setup.py install || error 
cd -


cd speculos
cmake -Bbuild -H. && make -C build/ || error "building speculos"
cd -

# this is needed for compiling the app - keep the package since speculos 
# only is compiled once but probably the app is compiled more often :see-no-evli:
apt-get -y install gcc-multilib g++-multilib


cd ledger-iota-app

# create env file
cat <<EOT >> env_nanos.sh
#!/bin/bash
export DEVICE=nanos
export BOLOS_SDK=/root/git/ledger-iota-app/dev/sdk/nanos-secure-sdk
export CLANGPATH=/root/Downloads/clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04/bin/
export GCCPATH=/root/Downloads/gcc-arm-none-eabi-5_3-2016q1/bin/
echo "nanos" > device.txt

EOT

sed 's|nanos|nanox|g' env_nanos.sh > env_nanox.sh

source env_$DEVICE.sh

# run.sh compiles the app anyways
#make || error "building iota ledger app"
cd -



echo
echo "build successfully"
echo
echo "start speculos with ./run.sh"


exit 0
