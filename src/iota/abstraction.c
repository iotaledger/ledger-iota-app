// methods that abstract from the used protocol

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "os.h"

#include "essence_stardust.h"
#include "abstraction.h"
#include "ui_common.h"

#include "macros.h"
#include "api.h"
#include "iota_io.h"
#include "iota/address.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

const uint8_t *get_output_address_ptr(const API_CTX *api, uint8_t index)
{
    MUST(index < api->essence.outputs_count);

    uint8_t *ret = 0;

    switch (api->protocol) {
    case PROTOCOL_STARDUST: {
        BASIC_OUTPUT *tmp = (BASIC_OUTPUT *)api->essence.outputs;
        // address follows the address_type in a pact struct
        ret = &tmp[index].address_type;
        break;
    }
    // not supported yet
    case PROTOCOL_NOVA: {
        THROW(SW_UNKNOWN);
    }
    default:
        THROW(SW_UNKNOWN);
        break;
    }
    return ret;
}

uint64_t get_output_amount(const API_CTX *api, uint8_t index)
{
    MUST(index < api->essence.outputs_count);

    uint64_t amount = 0;

    switch (api->protocol) {
    case PROTOCOL_STARDUST: {
        BASIC_OUTPUT *tmp = (BASIC_OUTPUT *)api->essence.outputs;
        memcpy(&amount, &tmp[index].amount, sizeof(uint64_t));
        break;
    }
    // not supported yet
    case PROTOCOL_NOVA: {
        THROW(SW_UNKNOWN);
    }
    default:
        THROW(SW_UNKNOWN);
        break;
    }
    return amount;
}

uint8_t address_encode_bech32(const API_CTX *api, const uint8_t *addr_with_type,
                              char *bech32, uint32_t bech32_max_length)
{
    const char *hrp;

    switch (api->coin) {
    case COIN_IOTA: {
        hrp = (api->app_mode & 0x80) ? COIN_HRP_IOTA_TESTNET : COIN_HRP_IOTA;
        break;
    }
    case COIN_SHIMMER: {
        hrp = (api->app_mode & 0x80) ? COIN_HRP_SHIMMER_TESTNET
                                     : COIN_HRP_SHIMMER;
        break;
    }
    default:
        THROW(SW_UNKNOWN);
        break;
    }

    MUST(address_encode_bech32_hrp(addr_with_type, bech32, bech32_max_length,
                                   hrp, strlen(hrp)));

    return 1;
}


uint8_t essence_parse_and_validate(API_CTX *api)
{
    switch (api->protocol) {
    case PROTOCOL_STARDUST: {
        MUST(essence_parse_and_validate_stardust(api));
        break;
    }
    // not supported yet
    case PROTOCOL_NOVA: {
        THROW(SW_UNKNOWN);
    }
    default:
        THROW(SW_UNKNOWN);
        break;
    }
    return 1;
}

uint8_t get_amount(const API_CTX *api, int index, char *dst, size_t dst_len)
{
    uint64_t amount;

    // amount > 0 enforced by validation
    MUST(amount = get_output_amount(api, index));

    format_value_full_decimals(dst, dst_len, amount);

    return 1;
}

