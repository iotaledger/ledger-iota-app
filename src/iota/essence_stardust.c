/*
 * message_validator.c
 *
 *  Created on: 15.10.2020
 *      Author: thomas
 */

// validation based on:
//  https://github.com/luca-moser/protocol-rfcs/blob/signed-tx-payload/text/0018-transaction-payload/0018-transaction-payload.md
//	https://github.com/lzpap/tips/blob/master/tips/TIP-0018/tip-0018.md

#include <stdint.h>
#include <string.h>

#include "os.h"
#include "cx.h"
#include "api.h"

#include "essence_stardust.h"
#include "internal_transfer.h"

#ifndef FUZZING
#include "iota_io.h"
#include "iota/ed25519.h"
#endif

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

// wrap-around safe check for addition
#define MUST_SUM_LOWER_THAN(a, b, sum)                                         \
    {                                                                          \
        if (!((a < sum) && (b < sum) && ((a + b) < sum))) {                    \
            return 0;                                                          \
        }                                                                      \
    }

static inline uint8_t get_uint32(const uint8_t *data, uint32_t *idx,
                                 uint32_t *v)
{
    MUST(*idx + sizeof(uint32_t) < API_BUFFER_SIZE_BYTES);
    memcpy(v, &data[*idx],
           sizeof(uint32_t)); // copy to avoid unaligned access
    *idx = *idx + sizeof(uint32_t);
    return 1;
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

static inline uint8_t get_uint8(const uint8_t *data, uint32_t *idx, uint8_t *v)
{
    MUST(*idx + sizeof(uint8_t) < API_BUFFER_SIZE_BYTES);
    *v = data[*idx];
    *idx = *idx + sizeof(uint8_t);
    return 1;
}


// --- validate inputs ---
static uint8_t validate_inputs(const uint8_t *data, uint32_t *idx,
                               UTXO_INPUT **inputs_ptr, uint16_t *inputs_count)
{
    // uses safe getter macro that returns an error in case of invalid access
    MUST(get_uint16(data, idx, inputs_count));

    // Inputs Count must be 0 < x < 127.
    // At least one input must be specified.
    MUST(*inputs_count >= INPUTS_MIN_COUNT &&
         *inputs_count <= INPUTS_MAX_COUNT_STARDUST);

    *inputs_ptr = (UTXO_INPUT *)&data[*idx];

    for (uint32_t i = 0; i < *inputs_count; i++) {
        MUST(*idx + sizeof(UTXO_INPUT) < API_BUFFER_SIZE_BYTES);

        UTXO_INPUT tmp;
        memcpy(&tmp, &data[*idx],
               sizeof(UTXO_INPUT)); // copy to avoid unaligned access

        // Input Type value must be 0, denoting an UTXO Input.
        MUST(tmp.input_type == INPUT_TYPE_UTXO);

        // Transaction Output Index must be 0 ≤ x < 127.
        MUST(/*tmp.transaction_output_id >= 0 &&*/ tmp.transaction_output_id <
             127);

        *idx = *idx + sizeof(UTXO_INPUT);
    }
    return 1;
}


// --- validate outputs ---
static uint8_t validate_outputs(const uint8_t *data, uint32_t *idx,
                                BASIC_OUTPUT **outputs_ptr,
                                uint16_t *outputs_count, uint8_t coin_type)
{
    // uses safe getter macro that returns an error in case of invalid access
    MUST(get_uint16(data, idx, outputs_count));

    // Outputs Count must be 0 < x < 127.
    // At least one output must be specified.
    MUST(*outputs_count >= OUTPUTS_MIN_COUNT &&
         *outputs_count <= OUTPUTS_MAX_COUNT_STARDUST);

    *outputs_ptr = (BASIC_OUTPUT *)&data[*idx];
    uint64_t total_amount = 0ull;

    for (uint32_t i = 0; i < *outputs_count; i++) {
        MUST(*idx + sizeof(BASIC_OUTPUT) < API_BUFFER_SIZE_BYTES);

        BASIC_OUTPUT tmp;
        memcpy(&tmp, &data[*idx],
               sizeof(BASIC_OUTPUT)); // copy to avoid unaligned access

        // Output Type must be 0, denoting a SigLockedSingleOutput.
        MUST(tmp.output_type == OUTPUT_TYPE_BASICOUTPUT);

        // Amount must be > 0.
        MUST(tmp.amount > 0);

        // Native Tokens Count must be 0
        MUST(tmp.native_tokens_count == 0);

        // always 1 condition
        MUST(tmp.unlock_conditions_count == 1);

        // always 0 for "Address Unlock Condition"
        MUST(tmp.unlock_condition_type == ADDRESS_UNLOCK_CONDITION);

        // always 0 for ED25519 addres
        MUST(tmp.address_type == ADDRESS_TYPE_ED25519);

        // no extra blocks
        MUST(tmp.blocks_count == 0);

        total_amount += tmp.amount;

        // detect overflows
        MUST(total_amount >= tmp.amount);

        *idx = *idx + sizeof(BASIC_OUTPUT);
    }

    // shimmer has inflation
    if (coin_type == COIN_IOTA) {
        MUST(total_amount <= TOTAL_AMOUNT_MAX);
    }

    return 1;
}


// validate payload
static uint8_t validate_payload(const uint8_t *data, uint32_t *idx)
{
    // uses safe getter macro that returns an error in case of invalid access
    uint32_t payload_length;
    MUST(get_uint32(data, idx, &payload_length));

    // wrap-around-safe
    MUST_SUM_LOWER_THAN(*idx, payload_length, API_BUFFER_SIZE_BYTES);

    *idx = *idx + payload_length;
    return 1;
}

// skip bytes
// we can't validate the content
static uint8_t validate_skip_bytes(uint32_t *idx, uint16_t bytes_to_skip)
{
    MUST(*idx + bytes_to_skip < API_BUFFER_SIZE_BYTES);

    *idx = *idx + bytes_to_skip;
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

// --- CHECK INPUTS FOR DUPLICATES ---
static uint8_t validate_inputs_duplicates(const UTXO_INPUT *inputs,
                                          uint16_t inputs_count)
{
    // at least 2 needed for check
    if (inputs_count < 2) {
        return 1;
    }

    // Every combination of Transaction ID + Transaction Output Index must be
    // unique in the inputs set.
    for (uint32_t i = 0; i < inputs_count; i++) {
        for (uint32_t j = i + 1; j < inputs_count; j++) {
            // we can check all bytes because first is the input_type that must
            // always be 0
            if (!memcmp(&inputs[i], &inputs[j], sizeof(UTXO_INPUT))) {
                return 0;
            }
        }
    }
    return 1;
}

static uint8_t essence_verify_remainder_address(
    uint32_t *bip32_path, BASIC_OUTPUT *outputs, uint32_t outputs_count,
    uint16_t remainder_index, API_REMAINDER_BIP32_INDEX *remainder_bip32)
{
    // check remainder_index
    MUST(remainder_index < outputs_count);

    // check bip32 index
    MUST(remainder_bip32->bip32_change & 0x80000000);
    MUST(remainder_bip32->bip32_index & 0x80000000);

    BASIC_OUTPUT tmp;

    explicit_bzero(&tmp, sizeof(BASIC_OUTPUT));

    // set bip32 index
    bip32_path[BIP32_ADDRESS_INDEX] = remainder_bip32->bip32_index;
    bip32_path[BIP32_CHANGE_INDEX] = remainder_bip32->bip32_change;

    // Block below cannot be fuzzed without going through crypto APIs
#ifndef FUZZING
    // address generate generates with address
    MUST(address_generate(bip32_path, BIP32_PATH_LEN, &tmp.address_type));

    // verify, the address is the same
    // relies on packed struct
    MUST(!memcmp(&outputs[remainder_index].address_type, &tmp.address_type,
                 ADDRESS_WITH_TYPE_SIZE_BYTES));
#else
    (void)outputs;
#endif
    return 1;
}

static void essence_hash(API_CTX *api)
{
    // Block below cannot be fuzzed without going through crypto APIs
#ifndef FUZZING
    cx_blake2b_t blake2b;
    cx_blake2b_init(&blake2b, BLAKE2B_SIZE_BYTES * 8);
    cx_hash(&blake2b.header, CX_LAST, api->data.buffer, api->essence.length,
            api->essence.hash, ADDRESS_SIZE_BYTES);
#else
    (void)api;
#endif
}

uint8_t essence_parse_and_validate_stardust(API_CTX *api)
{
    uint32_t idx = 0;

    // Transaction Essence Type value must be 0, denoting an Transaction
    // Essence. uses safe getter macro that returns an error in case of invalid
    // access
    uint8_t transaction_essence_type;
    MUST(get_uint8(api->data.buffer, &idx, &transaction_essence_type));

    MUST(transaction_essence_type == TRANSACTION_ESSENCE_TYPE_STARDUST);
    // network id
    MUST(validate_skip_bytes(&idx, sizeof(uint64_t)));

    // parse data
    MUST(validate_inputs(api->data.buffer, &idx, &api->essence.inputs,
                         &api->essence.inputs_count));

    // inputs commitment hash
    MUST(validate_skip_bytes(&idx, BLAKE2B_SIZE_BYTES));

    MUST(validate_outputs(api->data.buffer, &idx,
                          (BASIC_OUTPUT **)&api->essence.outputs,
                          &api->essence.outputs_count, api->coin));

    MUST(validate_payload(api->data.buffer, &idx));

    // save essence length
    api->essence.length = idx;

    // bip32 indices don't belong to the essence
    MUST(validate_inputs_bip32(api->data.buffer, &idx,
                               api->essence.inputs_count,
                               &api->essence.inputs_bip32_index));

    // save data length
    api->data.length = idx;

    // if remainder output, check the address
    if (api->essence.has_remainder) {
        MUST(essence_verify_remainder_address(
            api->bip32_path, (BASIC_OUTPUT *)api->essence.outputs,
            api->essence.outputs_count, api->essence.remainder_index,
            &api->essence.remainder_bip32));
#if 0
		// technically, this is valid ... so don't block it but keep this notice for documentation
		// essence with only remainder and no other output
		MUST(api->essence.outputs_count > 1);
#endif
    }

    // additional validation steps of parsed data
    MUST(validate_inputs_duplicates(api->essence.inputs,
                                    api->essence.inputs_count));

    // everything fine - calculate the hash
    essence_hash(api);

    // check if it's a sweeping transaction
    if (check_for_sweeping(api)) {
        api->essence.is_internal_transfer = 1;
    }

    return 1;
}
