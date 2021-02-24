FROM wollac/ledger-bolos AS test-app

ENV DEVICE=nanos

WORKDIR /root

# switch to non-interactive
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        git cmake qemu-user-static \
        python3-construct python3-jsonschema python3-mnemonic python3-pyelftools \
        gcc-arm-linux-gnueabihf libc6-dev-armhf-cross gdb-multiarch pkg-config \
        libssl-dev protobuf-compiler lld-7

RUN mkdir git

WORKDIR /root/git

COPY repository.url .
RUN git clone --recurse-submodules "$( cat repository.url )"

# build blue loader
#WORKDIR /root/git/ledger-iota-app/dev/blue-loader-python
#RUN python3 setup.py install

# build speculos simulator
WORKDIR /root/git/ledger-iota-app/dev/speculos
RUN cmake -Bbuild -H. && make -C build/ 

WORKDIR /root/git/ledger-iota-app

RUN echo    "#!/bin/bash\n" \
            "export DEVICE=nanos\n" \
            "export BOLOS_SDK=/root/git/ledger-iota-app/dev/sdk/nanos-secure-sdk\n" \
            "export ENV BOLOS_ENV=/opt/bolos\n" \
            "echo nanos > device.txt\n" > env_nanos.sh

RUN sed 's|nanos|nanox|g' env_nanos.sh > env_nanox.sh

WORKDIR /root/git/ledger-iota-app/tests
RUN gcc tests.c -o tests

# temporary until it is in the git repository
COPY test_headless.sh .
RUN chmod a+x test_headless.sh


#EXPOSE 9999
