#pragma once

#include "os.h"

uint8_t ed25519_public_key_to_bytes(uint8_t raw_pubkey[65], uint8_t output[32]);
