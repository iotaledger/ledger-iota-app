/*
 * blindsigning.c
 *
 *  Created on: 01.05.2022
 *      Author: thomas
 */

#include <stdint.h>
#include <string.h>

#include "os.h"
#include "cx.h"
#include "api.h"

#include "macros.h"
#include "blindsigning.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

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
static uint8_t
validate_signing_input(const uint8_t *data, uint32_t *idx,
                       uint8_t signing_input[SIGNING_INPUT_MAX_BYTES],
                       uint16_t signing_input_len)
{
    MUST(*idx + signing_input_len < API_BUFFER_SIZE_BYTES);

    // not much we can validate here since an hash are arbitrary bytes
    // copy from buffer to essence
    memcpy(signing_input, &data[*idx], signing_input_len);


    *idx = *idx + signing_input_len;
    return 1;
}

// validate if there are enough bip32 fragments
static uint8_t
validate_inputs_bip32(const uint8_t *data, uint32_t *idx, uint16_t inputs_count,
                      API_INPUT_BIP32_INDEX **inputs_bip32_indices)
{
    *inputs_bip32_indices = (API_INPUT_BIP32_INDEX *)&data[*idx];
    // check if there are as many bip32-paths as inputs
    for (uint32_t i = 0; i < inputs_count; i++) {
        MUST(*idx + sizeof(API_INPUT_BIP32_INDEX) < API_BUFFER_SIZE_BYTES);

        API_INPUT_BIP32_INDEX tmp;
        memcpy(&tmp, &data[*idx],
               sizeof(API_INPUT_BIP32_INDEX)); // copy to avoid unaligned access

        // check is MSBs set
        MUST(tmp.bip32_index & 0x80000000);
        MUST(tmp.bip32_change & 0x80000000);

        *idx = *idx + sizeof(API_INPUT_BIP32_INDEX);
    }
    return 1;
}


uint8_t parse_and_validate_blindsigning(API_CTX *api,
                                        uint16_t signing_input_len)
{
    uint32_t idx = 0;

    MUST((api.protocol == PROTOCOL_STARDUST &&
          signing_input_len == BLAKE2B_SIZE_BYTES) ||
         (api.protocol == PROTOCOL_NOVA &&
          (signing_input_len == SIGNING_INPUT_NOVA_32BYTE ||
           signing_input_len == SIGNING_INPUT_NOVA_64BYTE)));

    api->essence.signing_input_len = signing_input_len;

    // parse data
    MUST(validate_signing_input(api->data.buffer, &idx,
                                api->essence.signing_input, signing_input_len));

    // save essence length
    api->essence.length = idx;

    MUST(get_uint16(api->data.buffer, &idx, &api->essence.inputs_count));

    // Inputs Count must be 0 < x <= 128.
    // At least one input must be specified.
    MUST(api->essence.inputs_count >= INPUTS_MIN_COUNT &&
         api->essence.inputs_count <= INPUTS_MAX_COUNT_STARDUST);

    // bip32 indices don't belong to the essence
    MUST(validate_inputs_bip32(api->data.buffer, &idx,
                               api->essence.inputs_count,
                               &api->essence.inputs_bip32_index));

    // save data length
    api->data.length = idx;

    return 1;
}
