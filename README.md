# IOTA Chrysalis App for Ledger Hardware Wallets

[![license](https://img.shields.io/github/license/IOTA-Ledger/blue-app-iota.svg)](https://github.com/IOTA-Ledger/blue-app-iota/blob/master/LICENSE)


***It is strongly recommended to take a few minutes to read this document to make sure you fully understand how IOTA and the Ledger Hardware Wallet works, and how they interact together.***

## Table of contents

- [IOTA Chrysalis App for Ledger Hardware Wallets](#iota-chrysalis-app-for-ledger-hardware-wallets)
  - [Table of contents](#table-of-contents)
  - [Introduction](#introduction)
    - [Terminology](#terminology)
    - [Address Reuse](#address-reuse)
    - [IOTA Message](#iota-message)
    - [Parts of an IOTA Message](#parts-of-an-iota-message)
  - [How Ledger Hardware Wallets Work](#how-ledger-hardware-wallets-work)
  - [IOTA Specific Considerations for Ledger Hardware Wallets](#iota-specific-considerations-for-ledger-hardware-wallets)
    - [IOTA User-Facing App Functions](#iota-user-facing-app-functions)
      - [Functions](#functions)
      - [Display](#display)
    - [IOTA Security Concerns Relating to Ledger Hardware Wallets](#iota-security-concerns-relating-to-ledger-hardware-wallets)
    - [Limitations of Ledger Hardware Wallets](#limitations-of-ledger-hardware-wallets)
  - [FAQ](#faq)
      - [I lost my ledger, what should I do now?](#i-lost-my-ledger-what-should-i-do-now)
  - [Development](#development)
    - [Preparing development environment](#preparing-development-environment)
    - [Compile and load the IOTA Ledger app](#compile-and-load-the-iota-ledger-app)
  - [Specification](#specification)

---

## Introduction

IOTA is an unique cryptocurrency with specific design considerations that must be taken into account. This document will attempt to go over how the Ledger hardware wallet functions, and how to stay safe when using a Ledger to store IOTA.

### Terminology

*Seed:* A single secret key from that all private keys are derived (aka "*24 words*", [BIP39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki)).

*Private Key:* Private keys are derived from the seed ([BIP32](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki) with ED25519 curve, [BIP44](https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki). The private key is used to generate a public key. It is also used to prove you own said public key by means of creating a signature for a specific transaction.

*Public Key:* The public part of a private/public key pair.

*Address:* A hashed representation of the public key that is used to send coins to.

*Input:* The unspent transaction output (UTXO) from which coins are transfered.

*Output:* The address to which coins are transfered.

*Change/Remainder:* After sending funds to a 3rd party, all remaining funds on the account must be transferred to a new address - this is called the change or remainder address.

### Address Reuse

IOTA Chrysalis switched from quantum resistance signatures (Winternitz One-Time) to ED25519 signatures and from an account based model to an UTXO model. For this reason, address reuse no longer is problematic and IOTA is as secure as *other major crypto currencies*.

### IOTA Message

A message mainly is just a group of inputs and outputs. So if Bob has 10Mi, and wants to send Alice 3Mi, the message could look like this:

**input:** Bob -10Mi

**output:** Alice +3Mi

**output:** Bob +7Mi (change output / remainder)

This example highlights how IOTA handles messages. First it takes an input of 10Mi from Bob's address. It sends 3 of it to Alice, and it puts the remaining 7Mi on a new address belonging to Bob's seed.

All inputs require the private key to generate signatures which prove that you are the owner of the funds.

Because messages are *atomic units*, the network will never accept Bob's input of -10Mi without also accepting sending 3 to Alice, and 7 to a new address owned by Bob (in this example).

### Parts of an IOTA Message

An IOTA message is broken up into 2 halves. The first half is generating a message and creating signatures for it.

The second half is selecting 2 other messages to confirm, and performing the proof of work.

The Ledger is **only** responsible for generating signatures for a specific message. After that the host machine (or anybody else for that matter), can take the signatures and broadcast it to the network (however the signatures are only valid for the specific message).

## How Ledger Hardware Wallets Work

The Ledger Hardware Wallet works by deterministically generating IOTA keys based on your 24 word mnemonic (created when setting up the device).

Instead of storing the seed on your computer (which could be stolen by an attacker), the seed is stored on the Ledger device, and is never broadcast to the host machine it is connected to.

Instead, the host machine must ask the Ledger to provide the information (such as public keys or signatures). When creating messages, the host will generate an unsigned message and then send it to the Ledger device to be signed. The Ledger will then use the private keys associated with the inputs to generate unique signatures, and will then transfer **only the signatures** back to the host machine.

The host can then use these signatures (which are only valid for that specific message) to broadcast the message to the network. However as you can see, neither the seed, nor any of the private keys ever leave the device.

See [Ledger's documentation](http://ledger.readthedocs.io) to get more info about the inner workings of the Ledger Hardware Wallets.

## IOTA Specific Considerations for Ledger Hardware Wallets

### IOTA User-Facing App Functions

#### Functions

- *Display Address:* The wallet can ask the Ledger to display the address for a specific index on the seed. It **only** accepts the index, and subsequently generates the address itself and thus verifies that the address belongs to the Ledger. 

    *Note: Only the remainder address (shown as "Remainder") will be verified to belong to the Ledger. Outputs shown as "Send To" are not.*

- *Sign Transaction:* The wallet will generate a message for the user to approve before the Ledger signs it. **Ensure all amounts and addresses are correct before signing**. These signatures are then sent back to the host machine.

#### Display

**TODO**


### IOTA Security Concerns Relating to Ledger Hardware Wallets

All warnings on the Ledger are there for a reason, **MAKE SURE TO READ THEM** and refer to this document if there is any confusion.

- **Don't rush through signing a message.**
    Before a message is signed, all outputs of a message are shown on the display. The user can scroll through the individual outputs and finally choose whether to sign the message ("Accept") or not ("Reject").

    Outputs that go to 3rd party addresses are shown on the display with "Send To". The address to which the rest is sent is shown as "Remainder".

    The Remainder also shows the BIP32 path in addition to the amount. This address is calculated on the ledger and ensures that the rest of the IOTAs are sent to an address owned by the Leder Nano.

    If the input amount perfectly matches the output amount, there will be no remainder. **If there is no remainder, double check that you are sending the proper amount to the proper address because there is no remainder being sent back to your seed.**

### Limitations of Ledger Hardware Wallets

Due to the memory limitations of the Ledger Nano S/X, the messages have certain restrictions. The Ledger Nano S can only accept messages with about 17 inputs/outputs (e.g. 4 inputs, 13 outputs). The Ledger Nano X can accept messages with about 180 inputs/outputs. 

## FAQ

#### I lost my ledger, what should I do now?

Hopefully you wrote down your 24 recovery words and your optional passphrase in a safe place. If not, all your funds are lost.

If you did, the best solution is to buy a new Ledger and enter your 24 recovery words and your optional passphrase in the new device.<br>
After installation of the IOTA Ledger app, all your funds are restored. Take care to reinitialize your seed index correctly.

## Development

You either can

- run the app in a Ledger Nano S/X simulator or
- load the app on your read Ledger Nano S

In both cases, you find instructions here: [Ledger-IOTA-App-Docker Repository](docker)

### Preparing development environment

For active development it might be easier to install the development environment locally instead of using Docker:

- Clone this repo
- Ensure that all git submodules are initialized
    ```
    $ git submodule update --init --recursive
    ```
- Set up your development environment according to [Ledger Documentation - Getting Started](https://ledger.readthedocs.io/en/latest/userspace/getting_started.html).

### Compile and load the IOTA Ledger app

After the development environment has been installed, the app can be build and installed in the following way:

- Connect your Ledger to the PC and unlock it
- To load the app, be sure that the dashboard is opened in the Ledger
- Run the following commands to compile the app from source and load it
    ```
    $ make load
    ```
- Accept all the messages on the Ledger

## Specification

See: [APDU API Specification](docs/specification_chrysalis.md)

