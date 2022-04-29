#include "ui_common.h"

#include "ux.h"
#include "ux_layout_pb_ud.h"
#include "glyphs.h"

#include "flow_user_confirm.h"

extern flowdata_t flow_data;

// clang-format off

/*
Render data to the UI
 
"   Sign Hash    "
"18fac39809cf8952"
"1c72bdd00d6bf36b"
"76203cefe9997ee0"
"bb62f058b1e0d268"

*/
// clang-format on


static void populate_hash(short line_nr) {
    if (line_nr < 1 || line_nr > 4) {
        THROW(SW_UNKNOWN);
    }

    uint8_t chunk = (uint8_t) (line_nr - 1);

    char *dst = flow_data.flow_lines[line_nr];
    const uint8_t *src = &flow_data.api->essence.hash[chunk * LINE_WIDTH];

    // hash is 32 bytes
    const char* hex="0123456789abcdef";

    for (int i=0;i<16;i++) {
        *dst++ = hex[((*src & 0xf0) >> 4)];
        *dst++ = hex[*src++ & 0x0f];
    }
}
static void populate_data_blindsigning()
{
    flow_data.number_of_lines = 5;

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
            strcpy(flow_data.flow_lines[i], "   Sign Hash    ");
            break;
        case 1: // hash first line
        case 2: // hash second line
        case 3: // hash third line
        case 4: // hash fourth line
            populate_hash(i);
            break;
        }
        // always zero-terminate to be sure
        flow_data.flow_lines[i][LINE_WIDTH] = 0;
    }
}

void flow_start_blindsigning(const API_CTX *api, accept_cb_t accept_cb,
                            timeout_cb_t timeout_cb)
{
    flow_confirm_datasets(api, accept_cb, 0, timeout_cb,
                          &populate_data_blindsigning, FLOW_ACCEPT_REJECT, 1);
}