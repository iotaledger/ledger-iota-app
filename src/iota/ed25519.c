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

// reversing the public key and changing the last byte
uint8_t ed25519_public_key_to_bytes(const uint8_t raw_pubkey[65],
                                    uint8_t output[32])
{
    for (int i = 0; i < 32; i++) {
        output[i] = raw_pubkey[64 - i];
    }
    if (raw_pubkey[32] & 1) {
        output[31] |= 0x80;
    }
    return 1;
}
