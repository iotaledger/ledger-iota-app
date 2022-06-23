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
#include "bech32.h"
#include "address.h"
#include "ed25519.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

#include "debugprintf.h"

#define MUST(c)                                                                \
    {                                                                          \
        if (!(c)) {                                                            \
            return 0;                                                          \
        }                                                                      \
    }


uint8_t address_encode_bech32_hrp(const uint8_t *addr_with_type, char *bech32,
                                  uint32_t bech32_max_length, const char *hrp,
                                  const size_t hrp_len)
{
    // at least this space is needed - bech32_encode adds a zero-terminator
    // byte!
    if (bech32_max_length < ADDRESS_SIZE_BECH32_MAX + 1)
        return 0;

    uint32_t base32_length = ADDRESS_SIZE_BASE32;

    // encode address bytes to base32
    uint8_t base32[ADDRESS_SIZE_BASE32];

    uint8_t ret = base32_encode(base32, &base32_length, addr_with_type,
                                1 + ADDRESS_SIZE_BYTES);
    MUST(ret);

    // and encode base32 to bech32
    uint32_t bech32_length = bech32_max_length;
    ret = bech32_encode(bech32, &bech32_length, hrp, hrp_len, base32,
                        base32_length);
    MUST(ret);

    return 1;
}

uint8_t address_generate(uint32_t *bip32_path, uint32_t bip32_path_length,
                         uint8_t *addr)
{
    cx_ecfp_private_key_t pk;
    cx_ecfp_public_key_t pub;

    uint8_t ret = 0;
    BEGIN_TRY
    {
        TRY
        {
            ret =
                ed25519_get_key_pair(bip32_path, bip32_path_length, &pk, &pub);
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

    // ed25519_get_key_pair must succeed
    MUST(ret);

    // convert Ledger pubkey to pubkey bytes
    uint8_t pubkey_bytes[PUBKEY_SIZE_BYTES];

    ret = ed25519_public_key_to_bytes(&pub, pubkey_bytes);
    MUST(ret);

    //	debug_print_hex(pubkey_bytes, 32, 16);

    // set ed25519 address_type
    addr[0] = ADDRESS_TYPE_ED25519;

    cx_blake2b_t blake2b;
    cx_blake2b_init(&blake2b, BLAKE2B_SIZE_BYTES * 8);
    cx_hash(&blake2b.header, CX_LAST, pubkey_bytes, PUBKEY_SIZE_BYTES, &addr[1],
            ADDRESS_SIZE_BYTES);

    return 1;
}
