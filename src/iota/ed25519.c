#include <string.h>

#include "os.h"
#include "cx.h"

#include "macros.h"
#include "ed25519.h"

#include "constants.h"

#include "debugprintf.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

// bip-path
// 		0x2c'/coin_type'/account'/change'/index'


uint8_t ed25519_get_key_pair(uint32_t *bip32_path, uint32_t bip32_path_length,
                             cx_ecfp_private_key_t *pk,
                             cx_ecfp_public_key_t *pub)
{
    uint8_t keySeed[64];

    // getting the seed to derive and configuring it with SLIP10
    cx_err_t err = CX_OK;
    do {
        err = os_derive_bip32_with_seed_no_throw(
            HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip32_path, bip32_path_length,
            keySeed, NULL, (unsigned char *)"ed25519 seed", 12);
        if (err != CX_OK) {
            break;
        }

        // initializing the private key and public key instance
        // with selected curve ED25519
        err = cx_ecfp_init_private_key_no_throw(CX_CURVE_Ed25519, keySeed, 32,
                                                pk);
        if (err != CX_OK) {
            break;
        }

        err = cx_ecfp_init_public_key_no_throw(CX_CURVE_Ed25519, NULL, 0, pub);
        if (err != CX_OK) {
            break;
        }

        // generating the key pair
        err = cx_ecfp_generate_pair_no_throw(CX_CURVE_Ed25519, pub, pk, 1);
    } while (0);

    // resetting the variables to avoid leak
    explicit_bzero(keySeed, sizeof(keySeed));

    return err == CX_OK;
}

// reversing the public key and changing the last byte
uint8_t ed25519_public_key_to_bytes(cx_ecfp_public_key_t *pub, uint8_t *output)
{
    for (int i = 0; i < 32; i++) {
        output[i] = pub->W[64 - i];
    }
    if (pub->W[32] & 1) {
        output[31] |= 0x80;
    }
    return 1;
}

uint8_t ed25519_sign(cx_ecfp_private_key_t *privateKey, const uint8_t *msg,
                     uint32_t msg_length, unsigned char *output)
{
    MUST(cx_eddsa_sign_no_throw(privateKey, CX_SHA512, msg, msg_length,
                                  output, CX_SHA512_SIZE) == CX_OK);

	return 1;
}
