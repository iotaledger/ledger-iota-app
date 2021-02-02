# Testdata

The test data was generated using a reference application that records all APDU commands
and answers from the IOTA app running in a simulator.

Generating test data needs a running Speculos simulator. [Here](https://gitlab.com/ledger-iota-chrysalis/ledger-iota-app-docker) are
instructions how to setup one for the Ledger Nano S or X.

Afterwards, you can build and execute the reference test programm:

```
git clone https://gitlab.com/ledger-iota-chrysalis/ledger-iota-tests.rs
cargo build
./target/debug/rusttest -s -n -r reference.bin -f bin
```

The generated file can be replayed by using the mini C test program. This solution has the advantage, that automatic tests can be very solid because there are no dependencies that could break.




