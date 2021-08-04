# docker

With this docker files you can:
- run a simulator with a Leder Nano S or X
- compile the app and load it on a Nano S*

*: Loading an app on a Nano X outside the Ledger app-store is only supported with special developer versions that only could be borrowed from Ledger (with NDA). But for testing purposes, the Nano X is supported in the simulator.

# Building

For both cases you always have to build the docker image first.

Before builing the images, install docker:

```
$ sudo apt install docker.io
```

Normally, the `/var/run/docker.sock` file-socket has mode 2660 with `root:docker` ownership, that means only `root` and members of the `docker`-group can use the socket without `sudo`. You either can use `sudo` for each of the following commands, change the permissions temporary (`chmod 666 /var/run/docker.sock`) or add your user to the docker-group (`sudo usermod -A docker $SUDO_USER`; it is required to login again afterwards). For simplicity we just use `sudo`.

Clone the repository and run the build-script

```
$ git clone https://github.com/iotaledger/ledger-iota-app
$ cd ledger-iota-app
$ git submodule init
$ git submodule update --recursive
$ cd docker
$ sudo ./build_docker.sh
```


# Running the IOTA app in the simulator

After building the Docker container, the app can be run using following script:

```
$ sudo ./run_simulator.sh [-m (nanos*|nanox)]
```

The `-m` argument can be used to switch between Nano S and Nano X. The default is `nanos`.

After starting, the simulator listens on port 9999 and can be used without restrictions with the `ledger-iota.rs` library.


# Loading the IOTA app on a Ledger Nano S

To compile and load the IOTA app on a real Ledger Nano S use the following script:

```
$ sudo ./load_nanos.sh
```

