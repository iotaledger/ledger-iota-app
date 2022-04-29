/*
 * message_validator.c
 *
 *  Created on: 15.10.2020
 *      Author: thomas
 */

#include <stdint.h>
#include <string.h>

#include "os.h"
#include "cx.h"
#include "api.h"

#include "blindsigning.h"

// gcc doesn't know this and ledger's SDK cannot be compiled with Werror!
//#pragma GCC diagnostic error "-Werror"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

#define MUST(c)                                                                \
    {                                                                          \
        if (!(c)) {                                                            \
            return 0;                                                          \
        }                                                                      \
    }

static inline uint8_t get_uint16(const uint8_t *data, uint32_t *idx,
                                 uint16_t *v)
{
    MUST(*idx + sizeof(uint16_t) < API_BUFFER_SIZE_BYTES);
    memcpy(v, &data[*idx],
              sizeof(uint16_t)); // copy to avoid unaligned access
    *idx = *idx + sizeof(uint16_t);
    return 1;
}


// validate essence hash
static uint8_t validate_essence_hash(const uint8_t *data, uint32_t *idx, uint8_t essence_hash[ESSENCE_HASH_SIZE_BYTES])
{
    MUST(*idx + ESSENCE_HASH_SIZE_BYTES < API_BUFFER_SIZE_BYTES);

    // not much we can validate here since an hash are arbitrary bytes
    // copy from buffer to essence
    memcpy(&essence_hash[0], &data[*idx], ESSENCE_HASH_SIZE_BYTES);

    *idx = *idx + ESSENCE_HASH_SIZE_BYTES;
    return 1;
}

// validate if there are enough bip32 fragments
static uint8_t validate_inputs_bip32(const uint8_t *data, uint32_t *idx,
                              uint16_t inputs_count,
                              API_INPUT_BIP32_INDEX **inputs_bip32_indices)
{
    *inputs_bip32_indices = (API_INPUT_BIP32_INDEX *)&data[*idx];
    // check if there are as many bip32-paths as inputs
    for (uint32_t i = 0; i < inputs_count; i++) {
        MUST(*idx + sizeof(API_INPUT_BIP32_INDEX) < API_BUFFER_SIZE_BYTES);

        API_INPUT_BIP32_INDEX tmp;
        memcpy(
            &tmp, &data[*idx],
            sizeof(API_INPUT_BIP32_INDEX)); // copy to avoid unaligned access

        // check is MSBs set
        MUST(tmp.bip32_index & 0x80000000);
        MUST(tmp.bip32_change & 0x80000000);

        *idx = *idx + sizeof(API_INPUT_BIP32_INDEX);
    }
    return 1;
}

// we only need to check if we can return enough pubkeys + signatures
static uint8_t validate_count_signature_types(
    uint32_t idx, uint16_t inputs_count, uint8_t single_sign)
{
    // in single-sign mode no extra data is used for signatures
    if (single_sign) {
        return 1;
    }

    uint32_t bytes_needed = inputs_count * sizeof(SIGNATURE_BLOCK);

    MUST(idx + bytes_needed < API_BUFFER_SIZE_BYTES);
    return 1;
}

uint8_t essence_parse_and_validate_blindsigning(API_CTX *api) {
    uint32_t idx = 0;

    // parse data
    MUST(validate_essence_hash(api->data.buffer, &idx, &api->essence.hash[0]));

    // save essence length
    api->essence.length = idx;

    MUST(get_uint16(api->data.buffer, &idx, &api->essence.inputs_count));

    // Inputs Count must be 0 < x < 127.
    // At least one input must be specified.
    MUST(api->essence.inputs_count >= INPUTS_MIN_COUNT &&
         api->essence.inputs_count <= INPUTS_MAX_COUNT);

    // bip32 indices don't belong to the essence
    MUST(validate_inputs_bip32(api->data.buffer, &idx,
                               api->essence.inputs_count,
                               &api->essence.inputs_bip32_index));

    // save data length
    api->data.length = idx;

    // enough space for signature blocks?
    MUST(validate_count_signature_types(idx, api->essence.inputs_count, api->essence.single_sign_mode));

    return 1;
}