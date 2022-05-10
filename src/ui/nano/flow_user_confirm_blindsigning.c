#include "ui_common.h"

#include "ux.h"
#include "ux_layout_pb_ud.h"
#include "glyphs.h"

#include "flow_user_confirm.h"
#include "flow_user_confirm_blindsigning.h"

// gcc doesn't know this and ledger's SDK cannot be compiled with Werror!
//#pragma GCC diagnostic error "-Werror"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

#define FLOW_HASH_CHARS_PER_LINE   13

extern flowdata_t flow_data;

// clang-format off

/*
Render data to the UI
 
" Blind Signing  "
"18fac39809cf8952"
"1c72bdd00d6bf36b"
"76203cefe9997ee0"
"bb62f058b1e0d268"

*/
// clang-format on

static void generate_hash()
{
    const char *hex = "0123456789ABCDEF";

    // generate hash
    memset(flow_data.tmp, 0, sizeof(flow_data.tmp));

    const char *src = (const char *)flow_data.api->essence.hash;
    char *dst = flow_data.tmp;

    *dst++ = '0';
    *dst++ = 'x';
    for (uint8_t i = 0; i < 32; i++) {
        *dst++ = hex[*src >> 4];
        *dst++ = hex[*src++ & 0x0f];
    }
    *dst = 0;
}

static void populate_data_blindsigning()
{
    flow_data.number_of_lines = 6;

    generate_hash();

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

        switch (cy) {
        case 0: // show flow header
            strcpy(flow_data.flow_lines[i], "Blind Signing");
            break;
        case 1: // hash first line
        case 2: // hash second line
        case 3: // hash third line
        case 4: // hash fourth line
        case 5: // hash fifth line
                // display one char more in the last line to avoid an extra line
                // with a single character
            memcpy(flow_data.flow_lines[i],
                   &flow_data.tmp[(cy - 1) * FLOW_HASH_CHARS_PER_LINE],
                   FLOW_HASH_CHARS_PER_LINE + ((cy == 5) ? 1 : 0));
            break;
        }
        // always zero-terminate to be sure
        flow_data.flow_lines[i][LINE_WIDTH] = 0;
    }
}

void flow_start_blindsigning(const API_CTX *api, accept_cb_t accept_cb,
                             reject_cb_t reject_cb, timeout_cb_t timeout_cb)
{
    flow_confirm_datasets(api, accept_cb, reject_cb, timeout_cb,
                          &populate_data_blindsigning, FLOW_ACCEPT_REJECT, 1);
}
