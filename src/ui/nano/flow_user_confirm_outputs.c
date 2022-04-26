#include "ui_common.h"

#include "flow_user_confirm.h"

extern flowdata_t flow_data;

// clang-format off

/*
"   Remainder    "    "     Send To    "
"iota1q9u552alqq0"    "iota1q9u552alqq0"
"swuaqc0m3qytkm3q"    "swuaqc0m3qytkm3q"
"9k6j0cujh4ha78ax"    "9k6j0cujh4ha78ax"
"y7v8tej4gqzadmcs"    "y7v8tej4gqzadmcs"
"     Amount     "    "     Amount     "
"     2.050 Gi   "    "     2.050 Gi   "
"   BIP32 Path   "    ------------------
"   2c'/107a'/   "    ------------------
"   3d5b0a0a'/   "    ------------------
"   7e5faa72'    "    ------------------

*/
// clang-format on

void generate_bech32() {
    // clear buffer
    memset(flow_data.flow_bech32, 0, sizeof(flow_data.flow_bech32));

    // generate bech32 address including the address_type
    // since the struct is packed, the address follows directly the address_type
    address_encode_bech32(
        &flow_data.api->essence.outputs[flow_data.flow_outputs_index_current].address_type, flow_data.flow_bech32,
        sizeof(flow_data.flow_bech32));
}


static void populate_amount(short line_no, int read_index) {
    uint64_t amount;
    // avoid unaligned access
    memcpy(&amount,
                &flow_data.api->essence.outputs[read_index].amount,
                sizeof(uint64_t));

    if (flow_data.amount_toggle) { // full
        // max supply is 2779530283277761 - this fits nicely in one line
        // on the Ledger nano s always cut after the 16th char to not
        // make a page with a single 'i'.
        format_value_full(flow_data.flow_lines[line_no],
                            sizeof(flow_data.flow_lines[line_no]), amount);

        // (is done later anyways)
        // ui_output.lines[i][LINE_WIDTH]='\0';
    }
    else { // short
        format_value_short(flow_data.flow_lines[line_no],
                            sizeof(flow_data.flow_lines[line_no]), amount);
    }
}


static void populate_header(uint8_t type, short line_no) {
    switch (type) {
    case REMAINDER:
        strcpy(flow_data.flow_lines[line_no], "Remainder");
        break;
    case OUTPUT: {
        // how many non-remainder outputs are there?
        // this is safe because the case of an essence with only one
        // remainder address as output is already covered
        // (is_bip32_remainder flag would be set).
        int non_remainder_outputs =
            flow_data.api->essence.outputs_count -
            !!flow_data.api->essence.has_remainder;

        // more than one? Show with numbers on the UI
        if (non_remainder_outputs > 1) {
            snprintf(flow_data.flow_lines[line_no],
                        sizeof(flow_data.flow_lines[line_no]) - 1,
                        "Send To [%d]",
                        flow_data.flow_outputs_index_current + 1);
        }
        else {
            strcpy(flow_data.flow_lines[line_no], "Send To");
        }
        break;
    }
    default:
        THROW(SW_UNKNOWN);
        break;
    }
}

void populate_data_outputs()
{
    // default is normal output
    uint8_t type = OUTPUT;

    // translate the index of the data if needed
    // in user-confirm-mode reorder the datasets in a way that remainder always
    // is the last dateset
    int read_index = flow_data.flow_outputs_index_current;

    // does essence contain a remainder?
    if (flow_data.api->essence.has_remainder) {
        // is the remainder the last output in the essence?
        if (flow_data.api->essence.remainder_index ==
            flow_data.api->essence.outputs_count - 1) {
            // yes, but current index only is the remainder if it's the
            // remainder_index in case of an essence with only one remainder
            // output, this also would be true
            if (read_index == flow_data.api->essence.remainder_index) {
                type = REMAINDER;
            }
        }
        else {
            // no it's not - we have to take care about switching indices
            // for displaying on the UI current read_index is the last
            // dataset? -> display remainder current read_index is the
            // remainder? -> display last dataset
            if (read_index == flow_data.api->essence.outputs_count - 1) {
                read_index = flow_data.api->essence.remainder_index;
                type = REMAINDER;
            }
            else if (read_index == flow_data.api->essence.remainder_index) {
                read_index = flow_data.api->essence.outputs_count - 1;
            }
        }
    }

    generate_bech32();


    // there are two types of displays with different number of lines
    switch (type) {
    case REMAINDER:
        flow_data.number_of_lines = 8 + get_no_lines_bip32(flow_data.flow_bip32);
        break;
    case OUTPUT:
        flow_data.number_of_lines = 7;
        break;
    default:
        THROW(SW_UNKNOWN);
        break;
    }

    // fix ypos if needed
    if (flow_data.flow_scroll_ypos > flow_data.number_of_lines - 3) {
        flow_data.flow_scroll_ypos = flow_data.number_of_lines - 3;
    }


    // figure out if we have to reset the toggle flag
    uint8_t reset_toggle = 1;
    if (type == REMAINDER || type == OUTPUT) {
        // if amount not in the middle of the screen reset the toggle
        if (flow_data.flow_scroll_ypos ==
            4 /* for both output types the same constant*/) {
            reset_toggle = 0;
        }
    }

    if (reset_toggle) {
        flow_data.amount_toggle = 0;
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
        case 0: // show header of transaction
            populate_header(type, i);
            break;
        case 1: // bech32 first line
        case 2: // bech32 second line
        case 3: // bech32 third line
        case 4: // bech32 fourth line
            memcpy(flow_data.flow_lines[i], &flow_data.flow_bech32[(cy - 1) * LINE_WIDTH],
                   LINE_WIDTH);
            break;
        case 5: // show amount header
            strcpy(flow_data.flow_lines[i], "Amount");
            break;
        case 6: // show amount
            populate_amount(i, read_index);
            break;
        case 7: // show head of bip32 path
            strcpy(flow_data.flow_lines[i], "BIP32 Path");
            break;
        case 8:  // bip32 first line
        case 9:  // bip32 second line
        case 10: // bip32 third line
            format_bip32(flow_data.flow_bip32, cy - 8, flow_data.flow_lines[i],
                         sizeof(flow_data.flow_lines[i]));
            break;
        }
        // always zero-terminate to be sure
        flow_data.flow_lines[i][LINE_WIDTH] = 0;
    }
}
