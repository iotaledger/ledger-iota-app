#include <stdint.h>
#include <string.h>

#include "os.h"
#include "cx.h"
#include "api.h"

#include "essence.h"
#include "signing.h"

#ifndef FUZZING
#include "iota_io.h"
#include "iota/ed25519.h"
#endif

#define MUST(c)                                                                \
    {                                                                          \
        if (!(c)) {                                                            \
            return 0;                                                          \
        }                                                                      \
    }

// gcc doesn't know this and ledger's SDK cannot be compiled with Werror!
//#pragma GCC diagnostic error "-Werror"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

static uint16_t sign_signature(SIGNATURE_BLOCK *pBlock,
                               const uint8_t *essence_hash,
                               uint32_t *bip32_signing_path,
                               API_INPUT_BIP32_INDEX *input_bip32_index)
{
    cx_ecfp_private_key_t pk;
    cx_ecfp_public_key_t pub;

    // overwrite bip32_signing_path
    bip32_signing_path[BIP32_ADDRESS_INDEX] = input_bip32_index->bip32_index;
    bip32_signing_path[BIP32_CHANGE_INDEX] = input_bip32_index->bip32_change;

    uint32_t signature_length = 0;

    uint8_t ret = 0;
    BEGIN_TRY
    {
        TRY
        {
            // create key pair and convert pub key to bytes
            ret = ed25519_get_key_pair(bip32_signing_path, BIP32_PATH_LEN, &pk,
                                       &pub);

            ret = ret && ed25519_sign(&pk, essence_hash, BLAKE2B_SIZE_BYTES,
                                      pBlock->signature, &signature_length);
        }
        CATCH_OTHER(e)
        {
            THROW(e);
        }
        FINALLY
        {
            // always delete from stack
            explicit_bzero(&pk, sizeof(pk));
        }
    }
    END_TRY;

    // ed25519_get_key_pair and ed25519_sign must succeed
    MUST(ret);

    // length of signature must not be 0
    MUST(signature_length);

    MUST(ed25519_public_key_to_bytes(&pub, pBlock->public_key));

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
    if (signature_index && !api->essence.blindsigning) {
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

static uint16_t sign_regular(API_CTX *api, uint8_t *output,
                             uint32_t signature_index)
{
    MUST(signature_index < api->essence.inputs_count);

    API_INPUT_BIP32_INDEX input_bip32_index;
    API_INPUT_BIP32_INDEX *essence_inputs =
        (API_INPUT_BIP32_INDEX *)api->essence.inputs_bip32_index;

    memcpy(&input_bip32_index, &essence_inputs[signature_index],
           sizeof(API_INPUT_BIP32_INDEX)); // avoid unaligned access

    uint8_t reference_index = sign_get_reference_index(api, signature_index);

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

static uint16_t sign_blindsigning(API_CTX *api, uint8_t *output,
                                  uint32_t signature_index)
{
    MUST(signature_index < api->essence.inputs_count);

    // we need to convert the short version to the long version
    // with HARDENED flag set
    API_INPUT_BIP32_INDEX_SHORT input_bip32_index_short;
    API_INPUT_BIP32_INDEX_SHORT *essence_inputs =
        (API_INPUT_BIP32_INDEX_SHORT *)api->essence.inputs_bip32_index;

    memcpy(&input_bip32_index_short, &essence_inputs[signature_index],
           sizeof(API_INPUT_BIP32_INDEX_SHORT)); // avoid unaligned access

    API_INPUT_BIP32_INDEX input_bip32_index;

    input_bip32_index.bip32_index = input_bip32_index_short.bip32_index;
    input_bip32_index.bip32_change =
        input_bip32_index_short.bip32_change | 0x80000000;

    uint16_t signature_size = sign_signature_unlock_block(
        (SIGNATURE_UNLOCK_BLOCK *)output, api->essence.hash, api->bip32_path,
        &input_bip32_index);

    MUST(signature_size);

    return signature_size;
}

uint16_t sign(API_CTX *api, uint8_t *output,
                                  uint32_t signature_index)
{                                  
    uint16_t signature_size = 0;

    if (api->essence.blindsigning) {
        signature_size = sign_blindsigning(
            api, output, signature_index);
    }
    else {
        signature_size =
            sign_regular(api, output, signature_index);
    }
    MUST(signature_size);

    return signature_size;
}
