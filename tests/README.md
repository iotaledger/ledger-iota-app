# Testdata

The test data was generated using a reference application that records all APDU commands
and answers from the IOTA app running in a simulator.

Generating test data needs a running Speculos simulator. [Here](https://github.com/iotaledger/ledger-iota-app/docker) are
instructions how to setup one for the Ledger Nano S or X.

Afterwards, you can build and execute the reference test programm:

```
$ git clone https://github.com/iotaledger/ledger.rs
$ cargo build --examples
$ ./target/debug/examples/cli -s -n -r reference.bin -f bin
```

The generated file can be replayed by using the mini C test program. The test program doesn't have any special requirements or dependencies. It should just compile fine by simply building it with `gcc tests.c -o tests` .




