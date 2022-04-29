#include <stdint.h>
#include <string.h>

#include "os.h"
#include "cx.h"
#include "api.h"

#include "essence.h"

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

static uint16_t sign_signature(SIGNATURE_BLOCK *pBlock, uint16_t output_max_len,
                               const uint8_t *essence_hash,
                               uint32_t *bip32_signing_path,
                               API_INPUT_BIP32_INDEX *input_bip32_index)
{

    MUST(output_max_len >= sizeof(SIGNATURE_BLOCK));

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
    SIGNATURE_UNLOCK_BLOCK *pBlock, uint16_t output_max_len,
    const uint8_t *essence_hash, uint32_t *bip32_signing_path,
    API_INPUT_BIP32_INDEX *input_bip32_index)
{
    MUST(output_max_len >= sizeof(SIGNATURE_UNLOCK_BLOCK));

    pBlock->unlock_type = UNLOCK_TYPE_SIGNATURE;     // signature
    pBlock->signature_type = SIGNATURE_TYPE_ED25519; // ED25519

    // actually isn't needed because we know the SIGNATURE_BLOCK fits
    output_max_len -= 2;

    MUST(sign_signature(&pBlock->signature, output_max_len, essence_hash,
                        bip32_signing_path, input_bip32_index));

    return (uint16_t)sizeof(SIGNATURE_UNLOCK_BLOCK);
}

static uint16_t sign_reference_unlock_block(REFERENCE_UNLOCK_BLOCK *pBlock,
                                            uint16_t output_max_len,
                                            uint8_t signature_type)
{
    MUST(output_max_len >= sizeof(REFERENCE_UNLOCK_BLOCK));

    pBlock->reference = (uint16_t)signature_type & 0x007f;
    pBlock->unlock_type = UNLOCK_TYPE_REFERENCE; // reference

    return (uint16_t)sizeof(REFERENCE_UNLOCK_BLOCK);
}

static uint16_t sign_single_int(uint8_t *output, uint16_t output_max_len,
                                uint8_t *essence_hash,
                                uint32_t *bip32_signing_path,
                                uint8_t signature_type,
                                API_INPUT_BIP32_INDEX *input_bip32_index,
                                uint8_t is_blindsigning)
{
    if (is_blindsigning) {
        // in blindsigning mode, we have only "naked" signature blocks
        return sign_signature((SIGNATURE_BLOCK *)output, output_max_len,
                              essence_hash, bip32_signing_path,
                              input_bip32_index);
    }
    else {
        // in non-blindsigning mode, we have signature or reference blocks
        // if MSB is set, it's a signature unlock block
        if (signature_type & 0x80) {
            return sign_signature_unlock_block(
                (SIGNATURE_UNLOCK_BLOCK *)output, output_max_len, essence_hash,
                bip32_signing_path, input_bip32_index);
        }
        else {
            return sign_reference_unlock_block((REFERENCE_UNLOCK_BLOCK *)output,
                                               output_max_len, signature_type);
        }
    }
}

uint16_t sign_single(API_CTX *api, uint8_t *output, uint16_t output_max_len,
                     uint32_t signature_index)
{
    MUST(signature_index < api->essence.inputs_count);

    API_INPUT_BIP32_INDEX input_bip32_index;
    memcpy(&input_bip32_index,
           &api->essence.inputs_bip32_index[signature_index],
           sizeof(API_INPUT_BIP32_INDEX)); // avoid unaligned access

    uint16_t signature_size = sign_single_int(
        output, output_max_len, api->essence.hash, api->bip32_signing_path,
        api->essence.signature_types[signature_index], &input_bip32_index,
        !api->essence.blindsigning);

    MUST(signature_size);

    return signature_size;
}

