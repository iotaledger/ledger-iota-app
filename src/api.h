#ifndef API_H
#define API_H

#include <stdbool.h>
#include <stdint.h>

#include "os.h"

#include "iota_io.h"

#include "iota/address.h"

#define IO_STRUCT struct __attribute__((packed, may_alias))

typedef enum {
    EMPTY = 0,
    GENERATED_ADDRESSES = 1,
    VALIDATED_ESSENCE = 2,
    USER_CONFIRMED_ESSENCE = 3,
    SIGNATURES = 4,
    LOCKED = 5,
} DATA_TYPE;

typedef enum {
    APP_MODE_IOTA_CHRYSALIS = 0,
    APP_MODE_IOTA_STARDUST = 1,
    APP_MODE_SHIMMER_CLAIMING = 2,
    APP_MODE_SHIMMER = 3
} APP_MODE_TYPE;

typedef enum { PROTOCOL_CHRYSALIS = 0, PROTOCOL_STARDUST = 1 } PROTOCOL_TYPE;

typedef enum { COIN_IOTA = 0, COIN_SHIMMER = 1 } COIN_TYPE;

typedef IO_STRUCT
{
    uint8_t input_type;
    uint8_t transaction_id[TRANSACTION_ID_SIZE_BYTES];
    uint16_t transaction_output_id;
}
UTXO_INPUT;

typedef IO_STRUCT
{
    // 0 enforced in validation
    uint8_t output_type;

    // 0 enorced in validation
    uint8_t address_type;

    uint8_t address[ADDRESS_SIZE_BYTES];

    uint64_t amount;
}
SIG_LOCKED_SINGLE_OUTPUT;

typedef IO_STRUCT
{
    // 3 enforced in validation
    uint8_t output_type;

    uint64_t amount;

    // 0 enforced in validation
    uint8_t native_tokens_count;

    // "none" enforced in validation
    // NATIVE_TOKENS native_tokens

    // 1 enforced in validation
    uint8_t unlock_conditions_count;

    // 0 enforced in validation
    // (Address Unlock Condition)
    uint8_t unlock_condition_type;

    // 0 enforced in validation
    // (Ed25519 Address)
    uint8_t address_type;

    uint8_t pubkey_hash[ADDRESS_SIZE_BYTES];

    // 0 enforced in validation
    uint8_t blocks_count;

    // "none" enforced in validation
    // BLOCKS blocks;
}
BASIC_OUTPUT;

// used for blindisning
typedef IO_STRUCT
{
    uint8_t public_key[32];
    uint8_t signature[64];
}
SIGNATURE_BLOCK;

typedef IO_STRUCT
{
    uint8_t unlock_type;
    uint8_t signature_type;
    SIGNATURE_BLOCK signature;
}
SIGNATURE_UNLOCK_BLOCK;

typedef IO_STRUCT
{
    uint8_t unlock_type;
    uint16_t reference;
}
REFERENCE_UNLOCK_BLOCK;

// --- request and response structures ---
typedef IO_STRUCT
{
    uint32_t bip32_index;
    uint32_t bip32_change;
    uint32_t count;
}
API_GENERATE_ADDRESS_REQUEST;


typedef IO_STRUCT
{
    uint8_t app_version_major;
    uint8_t app_version_minor;
    uint8_t app_version_patch;
    uint8_t app_flags;
    uint8_t device;
    uint8_t debug;
}
API_GET_APP_CONFIG_RESPONSE;

typedef IO_STRUCT
{
    uint16_t data_length;
    uint8_t data_type;
    uint8_t data_block_size;
    uint8_t data_block_count;
}
API_GET_DATA_BUFFER_STATE_RESPONSE;

typedef IO_STRUCT
{
    uint16_t remainder_index;
    uint32_t remainder_bip32_index;
    uint32_t remainder_bip32_change;
}
API_PREPARE_SIGNING_REQUEST;

typedef IO_STRUCT
{
    uint32_t bip32_index;
    uint32_t bip32_change;
}
API_INPUT_BIP32_INDEX;

// same struct with different name
typedef API_INPUT_BIP32_INDEX API_REMAINDER_BIP32_INDEX;

// --- api-struct ---
typedef struct {
    // number of valid inputs
    uint16_t inputs_count;

    // size of usable data in bytes
    uint16_t length;

    // pointer to inputs
    // don't use this directly because data is unaligned to save space
    UTXO_INPUT *inputs;

    // number of valid outputs
    uint16_t outputs_count;

    // pointer to outputs
    // don't use this directly because data is unaligned to save space
    uint8_t *outputs;

    // pointer to BIP32 array for input addresses
    // don't use this directly because data is unaligned to save space
    API_INPUT_BIP32_INDEX *inputs_bip32_index;

    // flag for blindsigning
    uint8_t blindsigning;

    uint16_t remainder_index;
    API_REMAINDER_BIP32_INDEX remainder_bip32;

    // flag that signals if essence has a remainder
    uint8_t has_remainder;

    // flag that signals that it's a sweeping transaction
    uint8_t is_internal_transfer;

    // hash of the essence
    uint8_t hash[BLAKE2B_SIZE_BYTES];
} ESSENCE;

typedef struct {
    // block of input data
    uint8_t buffer[API_BUFFER_SIZE_BYTES];

    // type of data in buffer
    DATA_TYPE type;

    // length of data in buffer
    uint16_t length;
} API_DATA;

typedef struct {
    /// primary BIP32 path used for seed derivation
    uint32_t bip32_path[BIP32_PATH_LEN];

    // app_mode
    APP_MODE_TYPE app_mode;

    // protocol version
    PROTOCOL_TYPE protocol;

    // coin type
    COIN_TYPE coin;

    // buffer for api
    API_DATA data;

    // parsed essence
    ESSENCE essence;

    //  0 = interactive flows may be started
    //  1 = flows may not be started until reset
    // flag gets set when starting a flow and reset when the databuffer
    // is cleared
    int8_t flow_locked;

#ifdef APP_DEBUG
    uint8_t non_interactive_mode;
#endif

} API_CTX;

/// global context with everything related to the current api state
// extern API_CTX api;

/** @brief Clear and initialize the entire API context. */
void api_initialize(APP_MODE_TYPE app_mode, uint32_t account_index);

// get application configuration (flags and version)
uint32_t api_get_app_config(uint8_t is_locked);

// set account index
uint32_t api_set_account(uint8_t app_mode, const uint8_t *data, uint32_t len);

// reset api
uint32_t api_reset(void);

uint32_t api_show_flow(uint8_t flow);

// write block of data to device memory
uint32_t api_write_data_block(uint8_t block_number, const uint8_t *input_data,
                              uint32_t len);

// read block of data from device memory
uint32_t api_read_data_block(uint8_t block_number);

// get the state of the data buffeer
uint32_t api_get_data_buffer_state(void);

// clear data
void api_clear_data(void);

// clear data buffer
uint32_t api_clear_data_buffer(void);

uint32_t api_prepare_signing(uint8_t has_remainder, const uint8_t *data,
                             uint32_t len);

uint32_t api_prepare_blindsigning(void);

uint32_t api_user_confirm_essence(void);

uint32_t api_sign(uint8_t p1);

uint32_t api_generate_address(uint8_t show_on_screen, const uint8_t *data,
                              uint32_t len);

#ifdef APP_DEBUG
uint32_t api_dump_memory(uint8_t pagenr);
uint32_t api_set_non_interactive_mode(uint8_t mode);
#endif


#endif // API_H
