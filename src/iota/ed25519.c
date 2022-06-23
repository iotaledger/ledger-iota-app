#include <string.h>

#include "os.h"
#include "cx.h"

#include "ed25519.h"

#include "constants.h"

#include "debugprintf.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"


// bip-path
// 		0x2c'/0x107a'/account'/change'/index'


uint8_t ed25519_get_key_pair(uint32_t *bip32_path, uint32_t bip32_path_length,
                             cx_ecfp_private_key_t *pk,
                             cx_ecfp_public_key_t *pub)
{
    uint8_t keySeed[32];

    // getting the seed to derive and configuring it with SLIP10
    os_perso_derive_node_bip32_seed_key(
        HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip32_path, bip32_path_length,
        keySeed, NULL, (unsigned char *)"ed25519 seed", 12);

    // initializing the private key and public key instance
    // with selected curve ED25519
    cx_ecfp_init_private_key(CX_CURVE_Ed25519, keySeed, sizeof(keySeed), pk);
    cx_ecfp_init_public_key(CX_CURVE_Ed25519, NULL, 0, pub);

    // generating the key pair
    cx_ecfp_generate_pair(CX_CURVE_Ed25519, pub, pk, 1);

    // resetting the variables to avoid leak
    explicit_bzero(keySeed, sizeof(keySeed));

    return 1;
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
                     uint32_t msg_length, unsigned char *output,
                     uint32_t *output_length)
{
    *output_length = cx_eddsa_sign(privateKey, 0, CX_SHA512, msg, msg_length,
                                   NULL, 0, output, CX_SHA512_SIZE, NULL);

    if (*output_length != SIGNATURE_SIZE_BYTES) {
        return 0;
    }
    return 1;
}
