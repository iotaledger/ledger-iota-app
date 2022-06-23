#include <string.h>

#include "os.h"
#include "macros.h"

#include "iota_io.h"
#include "api.h"
#include "iota/constants.h"

#include "ui/nano/flow_user_confirm.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

extern unsigned char G_io_apdu_buffer[IO_APDU_BUFFER_SIZE];

void io_initialize()
{
    memset(G_io_apdu_buffer, 0, IO_APDU_BUFFER_SIZE);
    api_initialize(APP_MODE_INIT, 0);
}

void io_send(const void *ptr, unsigned int length, unsigned short sw)
{
    if (length > IO_APDU_BUFFER_SIZE - 2) {
        THROW(SW_UNKNOWN);
    }

    if ((uint32_t)G_io_apdu_buffer != (uint32_t)ptr) {
        memcpy(G_io_apdu_buffer, ptr, length);
    }

    G_io_apdu_buffer[length++] = sw >> 8;
    G_io_apdu_buffer[length++] = sw >> 0;

    // just send, the response is handled in the main loop
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, length);
}

uint8_t *io_get_buffer()
{
    return G_io_apdu_buffer;
}

unsigned int iota_dispatch(const uint8_t ins, const uint8_t p1,
                           const uint8_t p2, const uint8_t len,
                           const unsigned char *input_data, uint8_t is_locked)
{
    // commands that can be executed on a locked ledger
    // this is useful for initializing the ledger without errors
    if (is_locked && !(ins == INS_NONE || ins == INS_GET_APP_CONFIG ||
                       ins == INS_SET_ACCOUNT || ins == INS_RESET ||
                       ins == INS_CLEAR_DATA_BUFFER)) {
        THROW(SW_DEVICE_IS_LOCKED);
    }

    // check second byte for instruction
    switch (ins) {
    case INS_NONE:
        return 0;

    case INS_GET_APP_CONFIG:
        return api_get_app_config(is_locked);

    case INS_SET_ACCOUNT:
        return api_set_account(p1, input_data, len);

    case INS_RESET:
        return api_reset();

    case INS_WRITE_DATA_BLOCK:
        return api_write_data_block(p1, input_data, len);

    case INS_PREPARE_SIGNING:
        return api_prepare_signing(p2, input_data, len);

    case INS_PREPARE_BLINDSIGNING:
        return api_prepare_blindsigning();

    case INS_GENERATE_ADDRESS:
        return api_generate_address(p1, input_data, len);

    case INS_GET_DATA_BUFFER_STATE:
        return api_get_data_buffer_state();

    case INS_CLEAR_DATA_BUFFER:
        return api_clear_data_buffer();

    case INS_USER_CONFIRM_ESSENCE:
        return api_user_confirm_essence();

    case INS_SIGN_SINGLE:
        return api_sign(p1);

    case INS_READ_DATA_BLOCK:
        return api_read_data_block(p1);

    case INS_SHOW_FLOW:
        return api_show_flow(p1);

#ifdef APP_DEBUG
    case INS_DUMP_MEMORY:
        return api_dump_memory(p1);
    case INS_SET_NON_INTERACTIVE_MODE:
        return api_set_non_interactive_mode(p1);
#endif

        // unknown command ??
    default:
        THROW(SW_INS_NOT_SUPPORTED);
    }
    return 0; // to satisfy the compiler
}
