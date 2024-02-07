/*
 * address.c
 *
 *  Created on: 16.10.2020
 *      Author: thomas
 */

#include <string.h>

#include "os.h"
#include "cx.h"
#include "api.h"
#include "public_key.h"
#include "ed25519.h"
#include "macros.h"
#include "lib_standard_app/crypto_helpers.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

//#include "debugprintf.h"


uint8_t public_key_generate(uint32_t *bip32_path, uint32_t bip32_path_length,
                         uint8_t pubkey[PUBKEY_SIZE_BYTES])
{
    uint8_t raw_pubkey[65];

    MUST(bip32_derive_with_seed_get_pubkey_256(
             HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip32_path,
             bip32_path_length, raw_pubkey, NULL, CX_SHA512, NULL, 0) == CX_OK);

    MUST(ed25519_public_key_to_bytes(raw_pubkey, pubkey));
    return 1;
}
