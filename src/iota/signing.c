#include <stdint.h>
#include <string.h>

#include "os.h"
#include "cx.h"
#include "api.h"
#include "lib_standard_app/crypto_helpers.h"

#include "macros.h"
#include "signing.h"

#ifndef FUZZING
#include "iota_io.h"
#include "iota/ed25519.h"
#endif

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

static uint16_t sign_signature(SIGNATURE_BLOCK *pBlock,
                               const uint8_t *essence_hash,
                               uint32_t *bip32_signing_path,
                               API_INPUT_BIP32_INDEX *input_bip32_index)
{

    // overwrite bip32_signing_path
    bip32_signing_path[BIP32_ADDRESS_INDEX] = input_bip32_index->bip32_index;
    bip32_signing_path[BIP32_CHANGE_INDEX] = input_bip32_index->bip32_change;

    size_t signature_length = CX_SHA512_SIZE;

    MUST(bip32_derive_with_seed_eddsa_sign_hash_256(
             HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip32_signing_path,
             BIP32_PATH_LEN, CX_SHA512, essence_hash, BLAKE2B_SIZE_BYTES,
             pBlock->signature, &signature_length, NULL, 0) == CX_OK);

    MUST(signature_length == SIGNATURE_SIZE_BYTES);

    // get pubkey
    uint8_t raw_pubkey[65];

    MUST(bip32_derive_with_seed_get_pubkey_256(
             HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip32_signing_path,
             BIP32_PATH_LEN, raw_pubkey, NULL, CX_SHA512, NULL, 0) == CX_OK);

    // convert Ledger pubkey to pubkey bytes
    MUST(ed25519_public_key_to_bytes(raw_pubkey, pBlock->public_key));

    return (uint16_t)sizeof(SIGNATURE_BLOCK);
}

static uint16_t sign_signature_unlock_block(
    SIGNATURE_UNLOCK_BLOCK *pBlock, const uint8_t *essence_hash,
    uint32_t *bip32_signing_path, API_INPUT_BIP32_INDEX *input_bip32_index)
{
    pBlock->signature_type = SIGNATURE_TYPE_ED25519; // ED25519
    pBlock->unlock_type = UNLOCK_TYPE_SIGNATURE;     // signature

    MUST(sign_signature(&pBlock->signature, essence_hash, bip32_signing_path,
                        input_bip32_index));

    return (uint16_t)sizeof(SIGNATURE_UNLOCK_BLOCK);
}

static uint16_t sign_reference_unlock_block(REFERENCE_UNLOCK_BLOCK *pBlock,
                                            uint8_t reference_index)
{
    pBlock->reference = (uint16_t)reference_index;
    pBlock->unlock_type = UNLOCK_TYPE_REFERENCE; // reference

    return (uint16_t)sizeof(REFERENCE_UNLOCK_BLOCK);
}

static uint8_t sign_get_reference_index(API_CTX *api, uint32_t signature_index)
{
    API_INPUT_BIP32_INDEX *essence_inputs =
        (API_INPUT_BIP32_INDEX *)api->essence.inputs_bip32_index;

    // reference unlock block is only possible if signature_index != 0 and
    // it's not in blind_signing mode
    if (signature_index) {
        // check if it is a reference unlock block
        for (uint32_t i = 0; i < signature_index; i++) {
            // if there is a match, we found the reference block index
            if (!memcmp(&essence_inputs[signature_index], &essence_inputs[i],
                        sizeof(API_INPUT_BIP32_INDEX))) {
                return i;
            }
        }
    }
    return 0xff;
}

uint16_t sign(API_CTX *api, uint8_t *output, uint32_t signature_index)
{
    MUST(signature_index < api->essence.inputs_count);

    API_INPUT_BIP32_INDEX input_bip32_index;
    API_INPUT_BIP32_INDEX *essence_inputs =
        (API_INPUT_BIP32_INDEX *)api->essence.inputs_bip32_index;

    memcpy(&input_bip32_index, &essence_inputs[signature_index],
           sizeof(API_INPUT_BIP32_INDEX)); // avoid unaligned access

    uint8_t reference_index = 0xff;

    // no reference index in blindsigning mode
    if (!api->essence.blindsigning) {
        reference_index = sign_get_reference_index(api, signature_index);
    }

    uint16_t signature_size = 0;

    // 0xff if not a reference index block
    if (reference_index == 0xff) {
        signature_size = sign_signature_unlock_block(
            (SIGNATURE_UNLOCK_BLOCK *)output, api->essence.hash,
            api->bip32_path, &input_bip32_index);
    }
    else {
        signature_size = sign_reference_unlock_block(
            (REFERENCE_UNLOCK_BLOCK *)output, reference_index);
    }

    MUST(signature_size);

    return signature_size;
}
