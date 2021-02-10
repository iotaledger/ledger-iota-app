/*
 * flow.c
 *
 *  Created on: 21.10.2020
 *      Author: thomas
 */

#include "flow_user_confirm.h"

#include "ux.h"
#include "ux_layout_pb_ud.h"
//#include "ux_layout_nnnnn.h"
#include "glyphs.h"

#include "api.h"

#include "iota/constants.h"
#include "ui_common.h"

// gcc doesn't know this and ledger's SDK cannot be compiled with Werror!
//#pragma GCC diagnostic error "-Werror"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

extern uint32_t timer_events;

#define LINE_WIDTH 16

#define FLOW_USER_CONFIRM 0
#define FLOW_NEW_ADDRESS 1

//////////////////////////////////////////////////////////////////////

#if ADDRESS_SIZE_BECH32 != 64
#error "assumptions violated!"
#endif

#if LINE_WIDTH != 16
#error "assumptions violated!"
#endif


typedef unsigned int (*ux_callback_cb_t)(const bagl_element_t *e);
typedef void (*ux_fetch_data)();

// struct that contains the data to be displayed on the screen
// during confirming the outputs
struct {
    const API_CTX *api;

    // current index of output displayed
    short flow_outputs_index_current;

    // callbacks
    accept_cb_t accept_cb;
    reject_cb_t reject_cb;
    timeout_cb_t timeout_cb;

    // callbacks for next prev events
    ux_callback_cb_t next_cb;
    ux_callback_cb_t prev_cb;

    ux_callback_cb_t next_dataset_cb;
    ux_callback_cb_t prev_dataset_cb;

    // buffer for nnbnn layout
    char flow_lines[5][LINE_WIDTH + 1];
    int flow_scroll_ypos;

    // total number of lines
    uint8_t number_of_lines;

    // toggles between short and full format of amount
    uint8_t amount_toggle;

    // for UI-100s-timeout
    uint32_t flow_timer_start;

    // flag that indicates the flow is active
    uint8_t flow_active;

    uint8_t flow_mode;

    uint32_t flow_bip32[BIP32_PATH_LEN];
} flow_data;


//////////////////////////////////////////////////////////////////////

static void ui_confirm_output_step(short dir);
static void ui_confirm_output_value_toggle();

static unsigned int ux_confirm_output_next_cb(const bagl_element_t *e);
static unsigned int ux_confirm_output_prev_cb(const bagl_element_t *e);

static unsigned int ux_confirm_output_accept_prev_cb(const bagl_element_t *e);
static unsigned int ux_confirm_output_accept_cb(const bagl_element_t *e);
static unsigned int ux_confirm_output_reject_cb(const bagl_element_t *e);
static unsigned int ux_confirm_output_reject_next_cb(const bagl_element_t *e);


static unsigned int ux_confirm_new_address_ok_prev_cb(bagl_element_t *e);
static unsigned int ux_confirm_new_address_ok_next_cb(bagl_element_t *e);


//-------------------------------------------------------------------------


//------------------------------------------------------------------------
// clang-format off
UX_STEP_NOCB_POSTINIT(
    ux_confirm_output_prev,
    nn,
    ux_confirm_output_prev_cb(NULL),
    {
        "", ""
    }
);

UX_FLOW_CALL(
        ux_flow_confirm_output_value_toggle,
        ui_confirm_output_value_toggle()
)

// 2c'/107a'/ffffffff'/ffffffff'...
UX_STEP_TIMEOUT(
    ux_confirm_output_address_step,
    nnbnn,
    2500,
    ux_flow_confirm_output_value_toggle,
    {
        (const char*) &flow_data.flow_lines[0],
        (const char*) &flow_data.flow_lines[1],
        (const char*) &flow_data.flow_lines[2],
        (const char*) &flow_data.flow_lines[3],
        (const char*) &flow_data.flow_lines[4],
    }
);

UX_STEP_NOCB_POSTINIT(
    ux_confirm_output_next,
    nn,
    ux_confirm_output_next_cb(NULL),
    {
        "", ""
    }
);

//------------------------------------------------------------------------

UX_STEP_NOCB_POSTINIT(
    ux_confirm_output_accept_prev,
    nn,
    ux_confirm_output_accept_prev_cb(NULL),
    {
        "", ""
    }
);

UX_STEP_CB(
    ux_confirm_output_accept,
    pb_ud,
    ux_confirm_output_accept_cb(NULL),
    {
        &C_x_icon_check,
        "Accept"
    }
);

UX_STEP_CB(
    ux_confirm_output_reject,
    pb_ud,
    ux_confirm_output_reject_cb(NULL),
    {
        &C_x_icon_cross,
        "Reject"
    }
);

UX_STEP_NOCB_POSTINIT(
    ux_confirm_output_reject_next,
    nn,
    ux_confirm_output_reject_next_cb(NULL),
    {
        "", ""
    }
);

//------------------------------------------------------------------------

UX_STEP_NOCB_POSTINIT(
    ux_confirm_new_address_ok_prev,
    nn,
    ux_confirm_new_address_ok_prev_cb(NULL),
    {
        "", ""
    }
);

UX_STEP_CB(
    ux_confirm_new_address_ok,
    pb_ud,
    ux_confirm_output_accept_cb(NULL),
    {
        &C_x_icon_check,
        "Ok"
    }
);

UX_STEP_NOCB_POSTINIT(
    ux_confirm_new_address_ok_next,
    nn,
    ux_confirm_new_address_ok_next_cb(NULL),
    {
        "", ""
    }
);


UX_FLOW(
    ux_flow_confirm_output,
    &ux_confirm_output_prev,
    &ux_confirm_output_address_step,
    &ux_confirm_output_next,

    &ux_confirm_output_accept_prev,
    &ux_confirm_output_accept,
    &ux_confirm_output_reject,
    &ux_confirm_output_reject_next,

    &ux_confirm_new_address_ok_prev,
    &ux_confirm_new_address_ok,
    &ux_confirm_new_address_ok_next,

    FLOW_END_STEP
);
// clang-format on

//--- callbacks for ux components ---
static unsigned int ux_confirm_output_next_cb(const bagl_element_t *e)
{
    if (flow_data.next_cb) {
        return flow_data.next_cb(e);
    }
    return 0;
}

static unsigned int ux_confirm_output_prev_cb(const bagl_element_t *e)
{
    if (flow_data.prev_cb) {
        return flow_data.prev_cb(e);
    }
    return 0;
}


static unsigned int ux_confirm_output_accept_prev_cb(const bagl_element_t *e)
{
    UNUSED(e);
    flow_data.flow_outputs_index_current =
        flow_data.api->essence.outputs_count - 1;
    flow_data.flow_scroll_ypos =
        100; // gets changed to the right value in populate_data
    ui_confirm_output_step(0);
    return 0; // DO NOT REDRAW THE BUTTON
}

static unsigned int ux_confirm_output_reject_next_cb(const bagl_element_t *e)
{
    UNUSED(e);
    flow_data.flow_outputs_index_current = 0;
    flow_data.flow_scroll_ypos = -2;
    ui_confirm_output_step(0);
    //    ux_flow_init(0, ux_flow_confirm_output, &ux_confirm_output_reject);
    return 0; // DO NOT REDRAW THE BUTTON
}


static unsigned int ux_confirm_output_accept_cb(const bagl_element_t *e)
{
    UNUSED(e);
    if (flow_data.accept_cb) {
        flow_data.accept_cb();
    }
    flow_stop();
    return 0; // DO NOT REDRAW THE BUTTON
}

static unsigned int ux_confirm_output_reject_cb(const bagl_element_t *e)
{
    UNUSED(e);
    if (flow_data.reject_cb) {
        flow_data.reject_cb();
    }
    flow_stop();
    return 0; // DO NOT REDRAW THE BUTTON
}

// --- callback for handling transitions between datasets ---
static unsigned int user_confirm_next_dataset_cb(const bagl_element_t *e)
{
    UNUSED(e);
    // increment dataset-index
    flow_data.flow_outputs_index_current++;

    // already the last dataset?
    if (flow_data.flow_outputs_index_current >=
        flow_data.api->essence.outputs_count) {
        // yes: continue with accept/reject flow
        ux_flow_init(0, ux_flow_confirm_output, &ux_confirm_output_accept);
    }
    else {
        // no: reset y-pos and display new dataset
        flow_data.flow_scroll_ypos = -2;
        ui_confirm_output_step(0);
    }
    return 0;
}

static unsigned int user_confirm_prev_dataset_cb(const bagl_element_t *e)
{
    UNUSED(e);
    // decrement dataset-index
    flow_data.flow_outputs_index_current--;

    // was already the first dataset?
    if (flow_data.flow_outputs_index_current < 0) {
        // yes: continue with accept/reject flow
        ux_flow_init(0, ux_flow_confirm_output, &ux_confirm_output_reject);
        return 0;
    }
    else {
        flow_data.flow_scroll_ypos = 100; // gets fixed by populate_data
        ui_confirm_output_step(0);
        return 0;
    }
}

// --- callbacks for handling transitions from line to line within one dataset
// ---
static unsigned int user_confirm_next_cb(const bagl_element_t *e)
{
    // the last line of the current datasheet is shown in the middle of the
    // page?
    if (flow_data.flow_scroll_ypos == flow_data.number_of_lines - 3) {
        // yes: load next dataset
        if (flow_data.next_dataset_cb) {
            return flow_data.next_dataset_cb(e);
        }
    }
    // no: just advance line
    ui_confirm_output_step(1);
    return 0; // DO NOT REDRAW THE BUTTON
}


static unsigned int user_confirm_prev_cb(const bagl_element_t *e)
{
    // the first line of the current dataset is shown in the middle of the page?
    if (flow_data.flow_scroll_ypos == -2) {
        // yes: load previous dataset
        if (flow_data.prev_dataset_cb) {
            return flow_data.prev_dataset_cb(e);
        }
    }
    // no: just step back one line
    ui_confirm_output_step(-1);
    return 0; // DO NOT REDRAW THE BUTTON
}

// --- prev and next callbacks for new address mode ---
static unsigned int ux_confirm_new_address_ok_prev_cb(bagl_element_t *e)
{
    UNUSED(e);
    flow_data.flow_scroll_ypos =
        100; // gets changed to the right value in populate_data
    ui_confirm_output_step(0);
    return 0;
}
static unsigned int ux_confirm_new_address_ok_next_cb(bagl_element_t *e)
{
    UNUSED(e);
    flow_data.flow_scroll_ypos = -2;
    ui_confirm_output_step(0);
    return 0;
}

static unsigned int new_address_next_cb(const bagl_element_t *e)
{
    UNUSED(e);
    if (flow_data.flow_scroll_ypos == flow_data.number_of_lines - 3) {
        ux_flow_init(0, ux_flow_confirm_output, &ux_confirm_new_address_ok);
        return 0;
    }
    ui_confirm_output_step(1);
    return 0;
}

static unsigned int new_address_prev_cb(const bagl_element_t *e)
{
    UNUSED(e);
    if (flow_data.flow_scroll_ypos == -2) {
        ux_flow_init(0, ux_flow_confirm_output, &ux_confirm_new_address_ok);
        return 0;
    }
    ui_confirm_output_step(-1);
    return 0;
}

#define UNKNOWN 0
#define NEW_ADDRESS 1
#define REMAINDER 2
#define OUTPUT 3

/*
Render data to the UI

"  New Address   "    "   Remainder    "    "     Send To    "
"iota1q9u552alqq0"    "iota1q9u552alqq0"    "iota1q9u552alqq0"
"swuaqc0m3qytkm3q"    "swuaqc0m3qytkm3q"    "swuaqc0m3qytkm3q"
"9k6j0cujh4ha78ax"    "9k6j0cujh4ha78ax"    "9k6j0cujh4ha78ax"
"y7v8tej4gqzadmcs"    "y7v8tej4gqzadmcs"    "y7v8tej4gqzadmcs"
------------------    "     Amount     "    "     Amount     "
------------------    "     2.050 Gi   "    "     2.050 Gi   "
"   BIP32 Path   "    "   BIP32 Path   "    ------------------
"   2c'/107a'/   "    "   2c'/107a'/   "    ------------------
"   3d5b0a0a'/   "    "   3d5b0a0a'/   "    ------------------
"   7e5faa72'    "  "   7e5faa72'    "    ------------------

*/
static void populate_data()
{
    // default is normal output
    uint8_t type = OUTPUT;

    // translate the index of the data if needed
    // in user-confirm-mode reorder the datasets in a way that remainder always
    // is the last dateset
    int read_index = flow_data.flow_outputs_index_current;

    // find out type to show and switch indices if needed
    if (flow_data.flow_mode == FLOW_NEW_ADDRESS) {
        type = NEW_ADDRESS;
    }
    else { // FLOW_USER_CONFIRM
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
    }

    char tmp_bech32[ADDRESS_SIZE_BECH32 + 1] = {0}; // +1 zero terminator
    // generate bech32 address including the address_type
    // since the struct is packed, the address folows directly the address_type
    address_encode_bech32(
        &flow_data.api->essence.outputs[read_index].address_type, tmp_bech32,
        sizeof(tmp_bech32));

    // reset all lines to display
    os_memset(flow_data.flow_lines, 0, sizeof(flow_data.flow_lines));

    // there are three types of displays with different number of lines
    switch (type) {
    case NEW_ADDRESS:
        flow_data.number_of_lines = 7;
        break;
    case REMAINDER:
        flow_data.number_of_lines = 9;
        break;
    case OUTPUT:
        flow_data.number_of_lines = 7;
        break;
    default:
        THROW(SW_UNKNOWN);
        break;
    }

    // figure out, how many lines we need for a nice formatted bip32 path
    if (type == NEW_ADDRESS || type == REMAINDER) {
        for (int i = 1; i < 3; i++) {
            // with *0 no memory is changed
            if (format_bip32(flow_data.flow_bip32, i, (char *)0, 0)) {
                flow_data.number_of_lines++;
            }
        }
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

        // new address flow doesn't show amount
        if (type == NEW_ADDRESS) {
            // skip amount
            if (cy >= 5) {
                cy += 2;
            }
        }

        switch (cy) {
        case 0:
            switch (type) {
            case NEW_ADDRESS:
                strcpy(flow_data.flow_lines[i], "New Address");
                break;
            case REMAINDER:
                strcpy(flow_data.flow_lines[i], "Remainder");
                break;
            case OUTPUT: {
                // how many non-remainder outputs are there?
                // this is safe because the case of an essence with only one
                // remainder address as output is already is covered
                // (is_bip32_remainder flag would be set).
                int non_remainder_outputs =
                    flow_data.api->essence.outputs_count -
                    !!flow_data.api->essence.has_remainder;

                // more than one? Show with numbers on the UI
                if (non_remainder_outputs > 1) {
                    snprintf(flow_data.flow_lines[i],
                             sizeof(flow_data.flow_lines[i]) - 1,
                             "Send To [%d]",
                             flow_data.flow_outputs_index_current + 1);
                }
                else {
                    strcpy(flow_data.flow_lines[i], "Send To");
                }
                break;
            }
            default:
                THROW(SW_UNKNOWN);
                break;
            }
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            // split bech32 string into 4 parts of 16 characters each
            // This is safe because the bech32 string is 64 characters long and
            // the buffer is 65 bytes (incl zero terminator). Additionally, each
            // line of flow_data is 17 characters including it's own
            // zero-terminator. So, 4 parts with 16 bytes each can be safely
            // copied and displayed.
            memcpy(flow_data.flow_lines[i], &tmp_bech32[(cy - 1) * LINE_WIDTH],
                   LINE_WIDTH);
            break;
        case 5:
            strcpy(flow_data.flow_lines[i], "Amount");
            break;
        case 6: {
            uint64_t amount;
            // avoid unaligned access
            os_memcpy(&amount,
                      &flow_data.api->essence.outputs[read_index].amount,
                      sizeof(uint64_t));

            if (flow_data.amount_toggle) { // full
                // max supply is 2779530283277761 - this fits nicely in one line
                // on the Ledger nano s always cut after the 16th char to not
                // make a page with a single 'i'.
                format_value_full(flow_data.flow_lines[i],
                                  sizeof(flow_data.flow_lines[i]), amount);

                // (is done later anyways)
                // ui_output.lines[i][LINE_WIDTH]='\0';
            }
            else { // short
                format_value_short(flow_data.flow_lines[i],
                                   sizeof(flow_data.flow_lines[i]), amount);
            }
            break;
        }
        case 7:
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

// gets called by a timer to toggle the short and full view
static void ui_confirm_output_value_toggle()
{
    flow_data.amount_toggle = 1 - flow_data.amount_toggle;
    ui_confirm_output_step(0);
}


// use pre-init steps to load previous / next flow data
static void ui_confirm_output_step(short dir)
{
    if (dir == -1) {
        flow_data.flow_scroll_ypos = MAX(flow_data.flow_scroll_ypos--, -2);
    }
    else if (dir == 1) {
        flow_data.flow_scroll_ypos = MIN(flow_data.flow_scroll_ypos++,
                                         flow_data.number_of_lines - 1 - 2);
    }
    populate_data();
    ux_flow_init(0, ux_flow_confirm_output, &ux_confirm_output_address_step);
}

static void flow_start_int(const API_CTX *api, uint8_t mode,
                           accept_cb_t accept_cb, reject_cb_t reject_cb,
                           timeout_cb_t timeout_cb,
                           const uint32_t bip32[BIP32_PATH_LEN])
{
    os_memset(&flow_data, 0, sizeof(flow_data));
    os_memcpy(flow_data.flow_bip32, bip32, sizeof(flow_data.flow_bip32));

    flow_data.api = api;

    flow_data.accept_cb = accept_cb;
    flow_data.reject_cb = reject_cb;
    flow_data.timeout_cb = timeout_cb;

    flow_data.flow_timer_start = timer_events;

    flow_data.flow_active = 1;
    flow_data.flow_scroll_ypos =
        -2; // data[0] vertically in the middle of the screen

    flow_data.flow_mode = mode;

    ui_confirm_output_step(0);
}


void flow_start_user_confirm(const API_CTX *api, accept_cb_t accept_cb,
                             reject_cb_t reject_cb, timeout_cb_t timeout_cb,
                             const uint32_t bip32[BIP32_PATH_LEN])
{
    flow_start_int(api, FLOW_USER_CONFIRM, accept_cb, reject_cb, timeout_cb,
                   bip32);

    flow_data.next_cb = user_confirm_next_cb;
    flow_data.prev_cb = user_confirm_prev_cb;

    flow_data.next_dataset_cb = user_confirm_next_dataset_cb;
    flow_data.prev_dataset_cb = user_confirm_prev_dataset_cb;

    ui_confirm_output_step(0);
}

void flow_start_new_address(const API_CTX *api, accept_cb_t accept_cb,
                            timeout_cb_t timeout_cb,
                            const uint32_t bip32[BIP32_PATH_LEN])
{
    flow_start_int(api, FLOW_NEW_ADDRESS, accept_cb, 0, timeout_cb, bip32);

    flow_data.next_cb = new_address_next_cb;
    flow_data.prev_cb = new_address_prev_cb;

    ui_confirm_output_step(0);
}

void flow_init()
{
    os_memset(&flow_data, 0, sizeof(flow_data));
}

void flow_stop()
{
    flow_init();
    flow_main_menu();
}

void flow_timer_event()
{
    if (flow_data.flow_active) {
        if ((timer_events - flow_data.flow_timer_start) >= 1000 /* 100s */) {
            if (flow_data.timeout_cb) {
                flow_data.timeout_cb();
            }
            flow_stop();
        }
    }
}
