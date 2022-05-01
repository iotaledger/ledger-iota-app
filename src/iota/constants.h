/*
 * constants.h
 *
 *  Created on: 17.10.2020
 *      Author: thomas
 */

#ifndef SRC_IOTA_CONSTANTS_H_
#define SRC_IOTA_CONSTANTS_H_


//-----------------------------------------------------------------------------
// to trick eclipse because this is defined in the Makefile
// but is shown as error in the code editor
// doesn't have any effect when compiled with the Makefile
#ifndef APPVERSION

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define APPVERSION_MAJOR 0
#define APPVERSION_MINOR 6
#define APPVERSION_PATCH 0

#define APPVERSION                                                             \
    STR(APPVERSION_MAJOR) "." STR(APPVERSION_MINOR) "." STR(APPVERSION_PATCH)

#endif
//-----------------------------------------------------------------------------

#define NETWORK_MAINNET 1
#define NETWORK_TESTNET 0


#define TOTAL_AMOUNT_MAX 2779530283277761ull

#define INPUTS_MAX_COUNT 126
#define INPUTS_MIN_COUNT 1

#define OUTPUTS_MAX_COUNT 126
#define OUTPUTS_MIN_COUNT 1

#define DATA_BLOCK_SIZE 251 // cla + ins + p1 + p2 + p3 + data max 256 bytes

#if defined(TARGET_NANOX) || defined(TARGET_NANOS2)
#define DATA_BLOCK_COUNT 32 // approx 8kB
#else
#define DATA_BLOCK_COUNT 3 // 753Byte
#endif

#define API_BUFFER_SIZE_BYTES (DATA_BLOCK_COUNT * DATA_BLOCK_SIZE)

#define BIP32_PATH_LEN 5 // fixed
#define BIP32_COIN_INDEX 1
#define BIP32_ACCOUNT_INDEX 2
#define BIP32_CHANGE_INDEX 3
#define BIP32_ADDRESS_INDEX 4

#define COIN_IOTA       0x8000107a
#define COIN_SHIMMER    0x8000107b
#define COIN_TESTNET    0x80000001

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
#define ADDRESS_TYPE_ED25519    0

// signature type of ED25519 signature
#define SIGNATURE_TYPE_ED25519  0

// unlock-block types
#define UNLOCK_TYPE_SIGNATURE   0
#define UNLOCK_TYPE_REFERENCE   1

// input types
#define INPUT_TYPE_UTXO         0

// output types
#define OUTPUT_TYPE_SIGLOCKEDSINGLEOUTPUT   0


// following constants are valid with bech32 encoding (address_type included)
#define ADDRESS_SIZE_BASE32 ((ADDRESS_WITH_TYPE_SIZE_BYTES * 8 + 4) / 5)

#ifdef APP_DEBUG
#define ADDRESS_HRP "atoi"
#else
#define ADDRESS_HRP "iota"
#endif

#define ADDRESS_HRP_LENGTH 4

// Ed25519-based addresses will result in a Bech32 string of 63 characters.
#define ADDRESS_SIZE_BECH32 (ADDRESS_HRP_LENGTH + 1 + ADDRESS_SIZE_BASE32 + 6)


// API-constants
#define API_GENERATE_ADDRESSES_MAX_COUNT                                       \
    (API_BUFFER_SIZE_BYTES / ADDRESS_WITH_TYPE_SIZE_BYTES)

// very coarse estimation how many inputs the essence could have
// we can safely assume that we need at least 32 bytes per input
// this allows us to safe 100+ bytes in the essence struct
#define API_MAX_SIGNATURE_TYPES (API_BUFFER_SIZE_BYTES / BLAKE2B_SIZE_BYTES)

#endif /* SRC_IOTA_CONSTANTS_H_ */
