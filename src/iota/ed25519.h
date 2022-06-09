#pragma once

#include "os.h"

uint8_t ed25519_public_key_to_bytes(cx_ecfp_public_key_t *pub, uint8_t *output);
uint8_t ed25519_get_key_pair(uint32_t *bip32_path, uint32_t bip32_path_length,
                             cx_ecfp_private_key_t *pk,
                             cx_ecfp_public_key_t *pub);
uint8_t ed25519_sign(cx_ecfp_private_key_t *privateKey, const uint8_t *msg,
                     uint32_t msg_length, unsigned char *output,
                     uint32_t *output_length);
