// methods that abstract from the used protocol

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "os.h"

#include "essence.h"
#include "abstraction.h"

#include "api.h"
#include "iota_io.h"
#include "iota/address.h"

// gcc doesn't know this and ledger's SDK cannot be compiled with Werror!
//#pragma GCC diagnostic error "-Werror"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

extern API_CTX api;


uint8_t *get_output_address(const API_CTX *api, uint8_t index)
{
    MUST(index < api->essence.outputs_count);

    uint8_t *ret = 0;

    switch (api->protocol) {
    case PROTOCOL_STARDUST: {
        BASIC_OUTPUT *tmp = (BASIC_OUTPUT *)api->essence.outputs;
        ret = &tmp[index].address_type;
    }
    case PROTOCOL_CHRYSALIS: {
        SIG_LOCKED_SINGLE_OUTPUT *tmp =
            (SIG_LOCKED_SINGLE_OUTPUT *)api->essence.outputs;
        ret = &tmp[index].address_type;
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
    case PROTOCOL_CHRYSALIS: {
        SIG_LOCKED_SINGLE_OUTPUT *tmp =
            (SIG_LOCKED_SINGLE_OUTPUT *)api->essence.outputs;
        memcpy(&amount, &tmp[index].amount, sizeof(uint64_t));
        break;
    }
    default:
        THROW(SW_UNKNOWN);
        break;
    }
    return amount;
}

uint8_t address_encode_bech32(const uint8_t *addr_with_type, char *bech32,
                              uint32_t bech32_max_length)
{
    switch (api.coin) {
        case COIN_IOTA: {
            MUST(address_encode_bech32_hrp(addr_with_type, bech32, bech32_max_length, COIN_HRP_IOTA, strlen(COIN_HRP_IOTA)));
            break;
        }
        case COIN_SHIMMER: {
            MUST(address_encode_bech32_hrp(addr_with_type, bech32, bech32_max_length, COIN_HRP_SHIMMER, strlen(COIN_HRP_SHIMMER)));
            break;
        }
        default:
            THROW(SW_UNKNOWN);
            break;
    }
    return 1;
}


uint8_t essence_parse_and_validate(API_CTX *api)
{
    switch (api->protocol) {
    case PROTOCOL_STARDUST: {
        MUST(essence_parse_and_validate_stardust(api));
        break;
    }
    case PROTOCOL_CHRYSALIS: {
        MUST(essence_parse_and_validate_chryslis(api));
        break;
    }
    default:
        THROW(SW_UNKNOWN);
        break;
    }
    return 1;
}
