FROM wollac/ledger-bolos AS test-app

ENV DEVICE=nanos

# switch to non-interactive
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    cmake qemu-user-static \
    python3-construct python3-jsonschema python3-mnemonic python3-pyelftools \
    gcc-arm-linux-gnueabihf libc6-dev-armhf-cross gdb-multiarch pkg-config \
    libssl-dev protobuf-compiler lld-7

COPY . /root/git/ledger-iota-app/

WORKDIR /usr/bin
RUN ln -s clang-7 clang

# build speculos simulator
WORKDIR /root/git/ledger-iota-app/dev/speculos
RUN cmake -Bbuild -H. && make -C build/ 

WORKDIR /root/git/ledger-iota-app

RUN echo    "#!/bin/bash\n" \
    "export DEVICE=nanos\n" \
    "export BOLOS_SDK=/root/git/ledger-iota-app/dev/sdk/nanos-secure-sdk\n" \
    "unset BOLOS_ENV\n" \
    "export CLANGPATH=/usr/bin/\n" \
    "export GCCPATH=/usr/bin/\n" \
    "echo nanos > device.txt\n" > env_nanos.sh

RUN sed 's|nanos|nanox|g' env_nanos.sh > env_nanox.sh

WORKDIR /root/git/ledger-iota-app/tests
RUN gcc tests.c -o tests

#EXPOSE 9999
