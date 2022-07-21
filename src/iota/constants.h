/*
 * constants.h
 *
 *  Created on: 17.10.2020
 *      Author: thomas
 */

#pragma once

#include "os.h"

//-----------------------------------------------------------------------------

#define NETWORK_MAINNET 1
#define NETWORK_TESTNET 0


#define TOTAL_AMOUNT_MAX 2779530283277761ull

#define INPUTS_MAX_COUNT_CHRYSALIS 127
#define INPUTS_MAX_COUNT_STARDUST 128
#define INPUTS_MIN_COUNT 1

#define OUTPUTS_MAX_COUNT_CHRYSALIS 127
#define OUTPUTS_MAX_COUNT_STARDUST 128
#define OUTPUTS_MIN_COUNT 1

#define DATA_BLOCK_SIZE 251 // cla + ins + p1 + p2 + p3 + data max 256 bytes

#if defined(TARGET_NANOX) || defined(TARGET_NANOS2)
#define DATA_BLOCK_COUNT 32 // approx 8kB
#else
#define DATA_BLOCK_COUNT 6 // approx 1.5kB
#endif

#define API_BUFFER_SIZE_BYTES (DATA_BLOCK_COUNT * DATA_BLOCK_SIZE)

#define BIP32_PATH_LEN 5 // fixed
#define BIP32_COIN_INDEX 1
#define BIP32_ACCOUNT_INDEX 2
#define BIP32_CHANGE_INDEX 3
#define BIP32_ADDRESS_INDEX 4

#define BIP32_COIN_IOTA 0x8000107a
#define BIP32_COIN_SHIMMER 0x8000107b
#define BIP32_COIN_TESTNET 0x80000001

#if defined(APP_IOTA)
#define APP_MODE_INIT APP_MODE_IOTA_CHRYSALIS
#elif defined(APP_SHIMMER)
#define APP_MODE_INIT APP_MODE_SHIMMER
#else
#error unknown app
#endif


// the address is the ed25519 pub-key
#define ADDRESS_SIZE_BYTES 32
#define ADDRESS_WITH_TYPE_SIZE_BYTES 33

// the transaction_id is the blake2b hash of an address (without address-type)
#define TRANSACTION_ID_SIZE_BYTES 32

// size of pubkey
#define PUBKEY_SIZE_BYTES 32

// size of signature in bytes
#define SIGNATURE_SIZE_BYTES 64

// size of hash
#define BLAKE2B_SIZE_BYTES 32
#define ESSENCE_HASH_SIZE_BYTES 32

// address type of ED25519 addresses
#define ADDRESS_TYPE_ED25519 0

// signature type of ED25519 signature
#define SIGNATURE_TYPE_ED25519 0

// unlock-block types
#define UNLOCK_TYPE_SIGNATURE 0
#define UNLOCK_TYPE_REFERENCE 1

// stardust unlock types
#define ADDRESS_UNLOCK_CONDITION 0

// input types
#define INPUT_TYPE_UTXO 0

// output types
#define OUTPUT_TYPE_SIGLOCKEDSINGLEOUTPUT 0
#define OUTPUT_TYPE_BASICOUTPUT 3


#define TRANSACTION_ESSENCE_TYPE_STARDUST 1
#define TRANSACTION_ESSENCE_TYPE_CHRYSALIS 0


// following constants are valid with bech32 encoding (address_type included)
#define ADDRESS_SIZE_BASE32 ((ADDRESS_WITH_TYPE_SIZE_BYTES * 8 + 4) / 5)

#define COIN_HRP_IOTA "iota"
#define COIN_HRP_IOTA_TESTNET "atoi"
#define COIN_HRP_SHIMMER "smr"
#define COIN_HRP_SHIMMER_TESTNET "rms"

#define ADDRESS_HRP_LENGTH_MAX 4

// Ed25519-based addresses will result in a Bech32 string of 62/63 characters.
#define ADDRESS_SIZE_BECH32_MAX                                                \
    (ADDRESS_HRP_LENGTH_MAX + 1 + ADDRESS_SIZE_BASE32 + 6)


// API-constants
#define API_GENERATE_ADDRESSES_MAX_COUNT                                       \
    (API_BUFFER_SIZE_BYTES / ADDRESS_WITH_TYPE_SIZE_BYTES)

// very coarse estimation how many inputs the essence could have
// we can safely assume that we need at least 32 bytes per input
// this allows us to safe 100+ bytes in the essence struct
#define API_MAX_SIGNATURE_TYPES (API_BUFFER_SIZE_BYTES / BLAKE2B_SIZE_BYTES)
