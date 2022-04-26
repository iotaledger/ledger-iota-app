/*
 * flow.c
 *
 *  Created on: 21.10.2020
 *      Author: thomas
 */

#include "flow_user_confirm.h"
#include "flow_user_confirm_new_address.h"

#include "ux.h"
#include "ux_layout_pb_ud.h"
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


//////////////////////////////////////////////////////////////////////

#if ADDRESS_SIZE_BECH32 != 64
#error "assumptions violated!"
#endif

#if LINE_WIDTH != 16
#error "assumptions violated!"
#endif

flowdata_t flow_data;

static void ux_datasets_step(short dir);
static void ux_datasets_value_toggle();

static unsigned int ux_datasets_next_dataset_cb(const bagl_element_t *e);
static unsigned int ux_datasets_prev_dataset_cb(const bagl_element_t *e);

static unsigned int ux_datasets_next_cb(const bagl_element_t *e);
static unsigned int ux_datasets_prev_cb(const bagl_element_t *e);

static unsigned int ux_datasets_accept_prev_cb(const bagl_element_t *e);
static unsigned int ux_datasets_accept_cb(const bagl_element_t *e);
static unsigned int ux_datasets_reject_cb(const bagl_element_t *e);
static unsigned int ux_datasets_reject_next_cb(const bagl_element_t *e);

void populate_data();

//------------------------------------------------------------------------
// clang-format off
UX_STEP_NOCB_POSTINIT(
    ux_flow_datasets_prev,
    nn,
    ux_datasets_prev_cb(NULL),
    {
        "", ""
    }
);

UX_FLOW_CALL(
        ux_flow_datasets_value_toggle,
        ux_datasets_value_toggle()
)

// 2c'/107a'/ffffffff'/ffffffff'...
UX_STEP_TIMEOUT(
    ux_flow_datasets_step,
    nnbnn,
    2500,
    ux_flow_datasets_value_toggle,
    {
        (const char*) &flow_data.flow_lines[0],
        (const char*) &flow_data.flow_lines[1],
        (const char*) &flow_data.flow_lines[2],
        (const char*) &flow_data.flow_lines[3],
        (const char*) &flow_data.flow_lines[4],
    }
);

UX_STEP_NOCB_POSTINIT(
    ux_flow_datasets_next,
    nn,
    ux_datasets_next_cb(NULL),
    {
        "", ""
    }
);

//------------------------------------------------------------------------

UX_STEP_NOCB_POSTINIT(
    ux_flow_datasets_accept_prev,
    nn,
    ux_datasets_accept_prev_cb(NULL),
    {
        "", ""
    }
);

UX_STEP_CB(
    ux_flow_datasets_accept,
    pb_ud,
    ux_datasets_accept_cb(NULL),
    {
        &C_x_icon_check,
        "Accept"
    }
);

UX_STEP_CB(
    ux_flow_datasets_reject,
    pb_ud,
    ux_datasets_reject_cb(NULL),
    {
        &C_x_icon_cross,
        "Reject"
    }
);

UX_STEP_NOCB_POSTINIT(
    ux_flow_datasets_reject_next,
    nn,
    ux_datasets_reject_next_cb(NULL),
    {
        "", ""
    }
);

// -----------------------------------------------

UX_STEP_NOCB_POSTINIT(
    ux_flow_datasets_ok_prev,
    nn,
    ux_datasets_accept_prev_cb(NULL),
    {
        "", ""
    }
);

UX_STEP_CB(
    ux_flow_datasets_ok,
    pb_ud,
    ux_datasets_accept_cb(NULL),
    {
        &C_x_icon_check,
        "Ok"
    }
);

UX_STEP_NOCB_POSTINIT(
    ux_flow_datasets_ok_next,
    nn,
    ux_datasets_reject_next_cb(NULL),
    {
        "", ""
    }
);


UX_FLOW(
    ux_flow_datasets,
    &ux_flow_datasets_prev,
    &ux_flow_datasets_step,
    &ux_flow_datasets_next,

    &ux_flow_datasets_accept_prev,
    &ux_flow_datasets_accept,
    &ux_flow_datasets_reject,
    &ux_flow_datasets_reject_next,

    &ux_flow_datasets_ok_prev,
    &ux_flow_datasets_ok,
    &ux_flow_datasets_ok_next,

    FLOW_END_STEP
);
// clang-format on


//--- callbacks for ux components ---
static unsigned int ux_datasets_next_cb(const bagl_element_t *e)
{
    if (flow_data.next_cb) {
        return flow_data.next_cb(e);
    }
    return 0;
}

static unsigned int ux_datasets_prev_cb(const bagl_element_t *e)
{
    if (flow_data.prev_cb) {
        return flow_data.prev_cb(e);
    }
    return 0;
}

static unsigned int ux_datasets_accept_prev_cb(const bagl_element_t *e)
{
    UNUSED(e);
    flow_data.flow_outputs_index_current = flow_data.flow_dataset_count - 1;

    // gets changed to the right value in populate_data
    flow_data.flow_scroll_ypos = 100;

    ux_datasets_step(0);
    return 0; // DO NOT REDRAW THE BUTTON
}

static unsigned int ux_datasets_reject_next_cb(const bagl_element_t *e)
{
    UNUSED(e);
    flow_data.flow_outputs_index_current = 0;
    flow_data.flow_scroll_ypos = -2;
    ux_datasets_step(0);
    //    ux_flow_init(0, ux_flow_datasets, &ux_flow_datasets_reject);
    return 0; // DO NOT REDRAW THE BUTTON
}


static unsigned int ux_datasets_accept_cb(const bagl_element_t *e)
{
    UNUSED(e);
    if (flow_data.accept_cb) {
        flow_data.accept_cb();
    }
    flow_stop();
    return 0; // DO NOT REDRAW THE BUTTON
}

static unsigned int ux_datasets_reject_cb(const bagl_element_t *e)
{
    UNUSED(e);
    if (flow_data.reject_cb) {
        flow_data.reject_cb();
    }
    flow_stop();
    return 0; // DO NOT REDRAW THE BUTTON
}

// --- callback for handling transitions between datasets ---
static unsigned int ux_datasets_next_dataset_cb(const bagl_element_t *e)
{
    UNUSED(e);
    // increment dataset-index
    flow_data.flow_outputs_index_current++;

    // already the last dataset?
    if (flow_data.flow_outputs_index_current >= flow_data.flow_dataset_count) {
        // yes: continue with accept/reject flow
        if (flow_data.flow_type == FLOW_ACCEPT_REJECT) {
            ux_flow_init(0, ux_flow_datasets, &ux_flow_datasets_accept);
        }
        else {
            ux_flow_init(0, ux_flow_datasets, &ux_flow_datasets_ok);
        }
    }
    else {
        // no: reset y-pos and display new dataset
        flow_data.flow_scroll_ypos = -2;
        ux_datasets_step(0);
    }
    return 0;
}

static unsigned int ux_datasets_prev_dataset_cb(const bagl_element_t *e)
{
    UNUSED(e);
    // decrement dataset-index
    flow_data.flow_outputs_index_current--;

    // was already the first dataset?
    if (flow_data.flow_outputs_index_current < 0) {
        // yes: continue with accept/reject flow
        if (flow_data.flow_type == FLOW_ACCEPT_REJECT) {
            ux_flow_init(0, ux_flow_datasets, &ux_flow_datasets_reject);
        }
        else {
            ux_flow_init(0, ux_flow_datasets, &ux_flow_datasets_ok);
        }
        return 0;
    }
    else {
        flow_data.flow_scroll_ypos = 100; // gets fixed by populate_data
        ux_datasets_step(0);
        return 0;
    }
}

// --- callbacks for handling transitions from line to line within one dataset
// ---
static unsigned int ux_datasets_transition_next_cb(const bagl_element_t *e)
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
    ux_datasets_step(1);
    return 0; // DO NOT REDRAW THE BUTTON
}

static unsigned int ux_datasets_transition_prev_cb(const bagl_element_t *e)
{
    // the first line of the current dataset is shown in the middle of the page?
    if (flow_data.flow_scroll_ypos == -2) {
        // yes: load previous dataset
        if (flow_data.prev_dataset_cb) {
            return flow_data.prev_dataset_cb(e);
        }
    }
    // no: just step back one line
    ux_datasets_step(-1);
    return 0; // DO NOT REDRAW THE BUTTON
}

// gets called by a timer to toggle the short and full view
static void ux_datasets_value_toggle()
{
    flow_data.amount_toggle = 1 - flow_data.amount_toggle;
    ux_datasets_step(0);
}


// use pre-init steps to load previous / next flow data
static void ux_datasets_step(short dir)
{
    if (dir == -1) {
        flow_data.flow_scroll_ypos = MAX(flow_data.flow_scroll_ypos--, -2);
    }
    else if (dir == 1) {
        flow_data.flow_scroll_ypos = MIN(flow_data.flow_scroll_ypos++,
                                         flow_data.number_of_lines - 1 - 2);
    }
    populate_data();
    ux_flow_init(0, ux_flow_datasets, &ux_flow_datasets_step);
}

void populate_data()
{
    // reset all lines to display
    memset(flow_data.flow_lines, 0, sizeof(flow_data.flow_lines));

    if (flow_data.populate_cb) {
        flow_data.populate_cb();
    }
    else {
        THROW(SW_UNKNOWN);
    }
}


void flow_start(const API_CTX *api, accept_cb_t accept_cb,
                reject_cb_t reject_cb, timeout_cb_t timeout_cb,
                const uint32_t bip32[BIP32_PATH_LEN])
{
    memset(&flow_data, 0, sizeof(flow_data));
    memcpy(flow_data.flow_bip32, bip32, sizeof(flow_data.flow_bip32));

    flow_data.api = api;

    flow_data.accept_cb = accept_cb;
    flow_data.reject_cb = reject_cb;
    flow_data.timeout_cb = timeout_cb;

    flow_data.flow_timer_start = timer_events;

    flow_data.flow_active = 1;

    // data[0] vertically in the middle of the screen
    flow_data.flow_scroll_ypos = -2;
}

void flow_init()
{
    memset(&flow_data, 0, sizeof(flow_data));
}

void flow_stop()
{
    flow_init();
    flow_main_menu();
}
void flow_confirm_datasets(const API_CTX *api, accept_cb_t accept_cb,
                           reject_cb_t reject_cb, timeout_cb_t timeout_cb,
                           populate_cb_t populate_cb,
                           const uint32_t bip32[BIP32_PATH_LEN],
                           FLOW_TYPES flow_type, uint16_t dataset_count)
{
    flow_start(api, accept_cb, reject_cb, timeout_cb, bip32);

    flow_data.next_cb = ux_datasets_transition_next_cb;
    flow_data.prev_cb = ux_datasets_transition_prev_cb;

    flow_data.populate_cb = populate_cb;

    flow_data.next_dataset_cb = ux_datasets_next_dataset_cb;
    flow_data.prev_dataset_cb = ux_datasets_prev_dataset_cb;

    flow_data.flow_type = flow_type;
    flow_data.flow_dataset_count = dataset_count;

    ux_datasets_step(0);
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
