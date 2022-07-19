#include <stdint.h>
#include <string.h>

#include "os.h"

#include "iota/constants.h"
#include "api.h"
#include "abstraction.h"
#include "internal_transfer.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

#define MUST(c)                                                                \
    {                                                                          \
        if (!(c)) {                                                            \
            return 0;                                                          \
        }                                                                      \
    }

// checks if all coins remain on the wallet
// generate or each input bip32-path the address and compare it
// with the output address
// if a match is found, no coins are leaving the wallet
uint8_t check_for_internal_transfer(const API_CTX *api)
{
    // for sweeping only a single output and no remainder
    MUST(api->essence.outputs_count == 1 && !api->essence.has_remainder);

    // no internal transfer in SMR claiming
    MUST(api->app_mode != APP_MODE_SHIMMER_CLAIMING);

    // get the first (and only) output address
    const uint8_t *output = get_output_address_ptr(api, 0);

    API_INPUT_BIP32_INDEX *input_indices =
        (API_INPUT_BIP32_INDEX *)api->essence.inputs_bip32_index;

    uint32_t bip32_tmp_path[BIP32_PATH_LEN];
    memcpy(bip32_tmp_path, api->bip32_path, sizeof(bip32_tmp_path));

    for (uint16_t i = 0; i < api->essence.inputs_count; i++) {

        // set bip32 index
        // avoid unalighed access
        memcpy(&bip32_tmp_path[BIP32_ADDRESS_INDEX],
               &input_indices[i].bip32_index, sizeof(uint32_t));
        memcpy(&bip32_tmp_path[BIP32_CHANGE_INDEX],
               &input_indices[i].bip32_change, sizeof(uint32_t));

        uint8_t input[ADDRESS_WITH_TYPE_SIZE_BYTES];

        // address generate generates with address
        MUST(address_generate(bip32_tmp_path, BIP32_PATH_LEN, input));

        // check if input and output addresses match
        if (!memcmp(output, input, ADDRESS_WITH_TYPE_SIZE_BYTES)) {
            return 1;
        }
    }
    return 0;
}
