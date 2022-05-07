#include "ui_common.h"

#include "ux.h"
#include "ux_layout_pb_ud.h"
#include "glyphs.h"

#include "abstraction.h"
#include "flow_user_confirm.h"
#include "flow_user_confirm_new_address.h"

extern flowdata_t flow_data;

// gcc doesn't know this and ledger's SDK cannot be compiled with Werror!
//#pragma GCC diagnostic error "-Werror"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"


// clang-format off

/*
Render data to the UI
 
"Receive Address "
"iota1q9u552alqq0"
"swuaqc0m3qytkm3q"
"9k6j0cujh4ha78ax"
"y7v8tej4gqzadmcs"
"   BIP32 Path   "
"   2c'/107a'/   "
"   3d5b0a0a'/   "
"   7e5faa72'    "

*/
// clang-format on


static void generate_bech32()
{
    // clear buffer
    memset(flow_data.flow_bech32, 0, sizeof(flow_data.flow_bech32));

    // generate bech32 address including the address_type
    // we only have a single generated address in the buffer
    address_encode_bech32(
        &flow_data.api->data.buffer[0],
        flow_data.flow_bech32, sizeof(flow_data.flow_bech32));
}


static void populate_data_new_address()
{
    generate_bech32();

    flow_data.number_of_lines = 7 + get_no_lines_bip32(flow_data.api->bip32_path);

    // fix ypos if needed
    if (flow_data.flow_scroll_ypos > flow_data.number_of_lines - 3) {
        flow_data.flow_scroll_ypos = flow_data.number_of_lines - 3;
    }

    // iterate through all lines to display and populate them with the right
    // data
    for (short i = 0; i < 5; i++) {
        // calculate the real y line of the view-port
        short cy = i + flow_data.flow_scroll_ypos;

        // outside of the screen? then skip
        if (cy < 0 || cy > flow_data.number_of_lines - 1) {
            continue;
        }

        // clear line
        memset(flow_data.flow_lines[i], 0, sizeof(flow_data.flow_lines[i]));

        switch (cy) {
        case 0: // show flow header
            strcpy(flow_data.flow_lines[i],
                   (flow_data.api->bip32_path[BIP32_CHANGE_INDEX] & 0x1)
                       ? "New Remainder"
                       : "Receive Address");
            break;
        case 1: // bech32 first line
        case 2: // bech32 second line
        case 3: // bech32 third line
        case 4: // bech32 fourth line
        case 5: // bech32 fifth line
            memcpy(flow_data.flow_lines[i],
                   &flow_data.flow_bech32[(cy - 1) * BECH32_CHARS_PER_LINE], BECH32_CHARS_PER_LINE);
            break;
        case 6: // show bip32 path header
            strcpy(flow_data.flow_lines[i], "BIP32 Path");
            break;
        case 7: // bip32 first line
        case 8: // bip32 second line
        case 9: // bip32 third line
            format_bip32(flow_data.api->bip32_path, cy - 7, flow_data.flow_lines[i],
                         sizeof(flow_data.flow_lines[i]));
            break;
        }
        // always zero-terminate to be sure
        flow_data.flow_lines[i][LINE_WIDTH] = 0;
    }
}

void flow_start_new_address(const API_CTX *api, accept_cb_t accept_cb,
                            timeout_cb_t timeout_cb)
{
    flow_confirm_datasets(api, accept_cb, 0, timeout_cb,
                          &populate_data_new_address, FLOW_OK, 1);
}
