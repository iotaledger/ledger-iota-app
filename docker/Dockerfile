FROM wollac/ledger-bolos AS build-app

ENV DEVICE=nanos

# switch to non-interactive
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    libusb-dev libusb-1.0-0-dev libudev-dev cmake  qemu-user-static python3-pyqt5 \
    python3-construct python3-jsonschema python3-mnemonic python3-pyelftools \
    gcc-arm-linux-gnueabihf libc6-dev-armhf-cross gdb-multiarch pkg-config \
    libssl-dev protobuf-compiler lld-7 && \
    rm -rf /var/lib/apt/lists/*

COPY . /root/git/ledger-iota-app/

# temporary until base image was updated
WORKDIR /opt/bolos
RUN curl -L https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2017q4/gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2 --output gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2
RUN tar -xf gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2

WORKDIR /root/git/ledger-iota-app

# build blue loader
WORKDIR /root/git/ledger-iota-app/dev/blue-loader-python
RUN python3 setup.py install

# build speculos simulator
WORKDIR /root/git/ledger-iota-app/dev/speculos
RUN cmake -Bbuild -H. && make -C build/ 

WORKDIR /root/git/ledger-iota-app

RUN echo    "#!/bin/bash\n" \
    "export DEVICE=nanos\n" \
    "export BOLOS_SDK=/root/git/ledger-iota-app/dev/sdk/nanos-secure-sdk\n" \
    "unset BOLOS_ENV\n" \
    "export CLANGPATH=/opt/bolos/clang-arm-fropi/bin/\n" \
    "export GCCPATH=/opt/bolos/gcc-arm-none-eabi-7-2017-q4-major/bin/\n" \
    "echo nanos > device.txt\n" > env_nanos.sh

RUN sed 's|nanos|nanox|g' env_nanos.sh > env_nanox.sh


EXPOSE 9999
