/*
 * flow.c
 *
 *  Created on: 21.10.2020
 *      Author: thomas
 */

#include "ux.h"

#include "glyphs.h"

#include "api.h"
#include "flow_user_confirm_transaction.h"
#include "abstraction.h"

#include "iota/constants.h"
#include "ui_common.h"

// gcc doesn't know this and ledger's SDK cannot be compiled with Werror!
//#pragma GCC diagnostic error "-Werror"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

#define UNKNOWN 0
#define NEW_ADDRESS 1
#define REMAINDER 2
#define OUTPUT 3

#define MUST_THROW(c)                                                          \
    {                                                                          \
        if (!(c)) {                                                            \
            THROW(SW_UNKNOWN);                                                 \
        }                                                                      \
    }

extern flowdata_t flow_data;

static void cb_value_toggle();

static unsigned int cb_accept(const bagl_element_t *e);
static unsigned int cb_reject(const bagl_element_t *e);

static void cb_address_preinit();
static void cb_amount_preinit();
static void cb_output_preinit();
static void cb_bip32_preinit();

static void cb_switch();
static void cb_back();

static void cb_next_dataset();
static void cb_prev_dataset();

static void get_type_and_read_index();


//------------------------------------------------------------------------
// clang-format off

static UX_FLOW_CALL(
        ux_flow_datasets_value_toggle,
        cb_value_toggle()
)

// review output [...]
static UX_STEP_NOCB_INIT(
    ux_step_review,
    bb,
    cb_output_preinit(),
    {
        "Review", (const char*) flow_data.data
    }
);

static UX_STEP_NOCB_INIT(
    ux_step_amount,
    bn,
    cb_amount_preinit(),
    {
        "Amount SMR", (const char*) flow_data.data
    }
);

static UX_STEP_NOCB_INIT(
    ux_step_address,
    bn_paging,
    cb_address_preinit(),
    {
        "Address", (const char*) flow_data.data
    }
);

#ifdef TARGET_NANOS    
static UX_STEP_NOCB_INIT(
    ux_step_remainder,
    bn_paging,
    cb_bip32_preinit(),
    {
        "BIP32 Path", (const char*) flow_data.data
    }
);
#else
static UX_STEP_NOCB_INIT(
    ux_step_remainder,
    bn,
    cb_bip32_preinit(),
    {
        "BIP32 Path", (const char*) flow_data.data
    }
);
#endif

static UX_STEP_INIT(
    ux_step_switch,
    NULL,
    NULL,
    cb_switch()
);

static UX_STEP_INIT(
    ux_step_data_next,
    NULL,
    NULL,
    cb_next_dataset()
);

static UX_STEP_INIT(
    ux_step_data_prev,
    NULL,
    NULL,
    cb_prev_dataset()
);

static UX_STEP_INIT(
    ux_step_back,
    NULL,
    NULL,
    cb_back()
);
//------------------------------------------------------------------------

static UX_STEP_CB(
    ux_step_accept,
    pb,
    cb_accept(NULL),
    {
        &C_x_icon_check,
        "Accept"
    }
);

static UX_STEP_CB(
    ux_step_reject,
    pb,
    cb_reject(NULL),
    {
        &C_x_icon_cross,
        "Reject"
    }
);

static UX_FLOW(
    ux_flow_base,
    &ux_step_data_prev,
    &ux_step_review,
    &ux_step_address,
    &ux_step_amount,
    &ux_step_switch,
    &ux_step_data_next,
    FLOW_LOOP
);

static UX_FLOW(
    ux_flow_has_remainder,
    &ux_step_back,
    &ux_step_remainder,
    &ux_step_data_next,
    FLOW_LOOP
);

static UX_FLOW(
    ux_flow_has_accept_reject,
    &ux_step_back,
    &ux_step_accept,
    &ux_step_reject,
    &ux_step_data_next,
    FLOW_LOOP
);

static UX_FLOW(
    ux_flow_has_remainder_accept_reject,
    &ux_step_back,
    &ux_step_remainder,
    &ux_step_accept,
    &ux_step_reject,
    &ux_step_data_next,
    FLOW_LOOP
);

// clang-format on


//--- callbacks for ux components ---
typedef struct {
    const ux_flow_step_t *const *flow;
    const ux_flow_step_t *const step;
} jump_table_t;

static const jump_table_t jump_table_switch[4] = {
    // !last && !remainder
    {ux_flow_base, &ux_step_data_next},
    // !last &&  remainder
    {ux_flow_has_remainder, &ux_step_remainder},
    //  last && !remainder
    {ux_flow_has_accept_reject, &ux_step_accept},
    //  last &&  remainder
    {ux_flow_has_remainder_accept_reject, &ux_step_remainder}};

static const jump_table_t jump_table_prev[4] = {
    // !last && !remainder
    {ux_flow_base, &ux_step_amount},
    // !last &&  remainder
    {ux_flow_has_remainder, &ux_step_remainder},
    //  last && !remainder
    {ux_flow_has_accept_reject, &ux_step_reject},
    //  last &&  remainder
    {ux_flow_has_remainder_accept_reject, &ux_step_reject}};

static void cb_switch()
{
    uint8_t remainder = !!(flow_data.type == REMAINDER);
    uint8_t last = !!(flow_data.flow_outputs_index_current ==
                      flow_data.api->essence.outputs_count - 1);

    uint8_t m = (last << 1) | remainder;
    ux_flow_init(0, jump_table_switch[m].flow, jump_table_switch[m].step);
#if 0    
    switch (m) {
        case 0x0:   // no remainder, not last
            ux_flow_init(0, ux_flow_base, &ux_step_data_next);
            break;
        case 0x1:   // remainder, not last
            ux_flow_init(0, ux_step_remainder, &ux_step_remainder);
            break;
        case 0x2:   // no remainder, last
            ux_flow_init(0, ux_flow_accept_reject, &ux_step_accept);
            break;
        case 0x3:   // remainder, last
            ux_flow_init(0, ux_flow_remainder_accept_reject, &ux_step_remainder);
            break;
        default:
            THROW(SW_UNKNOWN);
    }
#endif
}

static void cb_next_dataset()
{
    flow_data.flow_outputs_index_current++;
    if (flow_data.flow_outputs_index_current >=
        flow_data.api->essence.outputs_count) {
        flow_data.flow_outputs_index_current = 0;
    }
    get_type_and_read_index();
    ux_flow_init(0, ux_flow_base, &ux_step_review);
}

static void cb_prev_dataset()
{
    flow_data.flow_outputs_index_current--;
    if (flow_data.flow_outputs_index_current < 0) {
        flow_data.flow_outputs_index_current =
            flow_data.api->essence.outputs_count - 1;
    }
    get_type_and_read_index();

    uint8_t remainder = !!(flow_data.type == REMAINDER);
    uint8_t last = !!(flow_data.flow_outputs_index_current ==
                      flow_data.api->essence.outputs_count - 1);

    uint8_t m = (last << 1) | remainder;
    ux_flow_init(0, jump_table_prev[m].flow, jump_table_prev[m].step);

#if 0
    switch (m) {
        case 0x0:   // no remainder, not last
            ux_flow_init(0, ux_flow_base, &ux_step_amount);
            break;
        case 0x1:   // remainder, not last
            ux_flow_init(0, ux_flow_has_remainder, &ux_step_remainder);
            break;
        case 0x2:   // no remainder, last
            ux_flow_init(0, ux_flow_has_accept_reject, &ux_step_reject);
            break;
        case 0x3:   // remainder, last
            ux_flow_init(0, ux_flow_has_remainder_accept_reject, &ux_step_reject);
            break;
        default:
            THROW(SW_UNKNOWN);
    }
#endif
}

static void cb_back()
{
    ux_flow_init(0, ux_flow_base, &ux_step_amount);
}

static unsigned int cb_accept(const bagl_element_t *e)
{
    UNUSED(e);
    if (flow_data.accept_cb) {
        flow_data.accept_cb();
    }
    flow_stop();
    return 0;
}

static unsigned int cb_reject(const bagl_element_t *e)
{
    UNUSED(e);
    if (flow_data.reject_cb) {
        flow_data.reject_cb();
    }
    flow_stop();
    return 0;
}

static void cb_amount_preinit()
{
    // clear buffer
    memset(flow_data.data, 0, sizeof(flow_data.data));

    MUST_THROW(get_amount(flow_data.api, flow_data.read_index, flow_data.data,
                          sizeof(flow_data.data), flow_data.amount_toggle));
}

static void cb_bip32_preinit()
{
    // clear buffer
    memset(flow_data.data, 0, sizeof(flow_data.data));

    format_bip32_with_line_breaks(flow_data.api->bip32_path, flow_data.data,
                                  sizeof(flow_data.data));
}

static void cb_address_preinit()
{
    // clear buffer
    memset(flow_data.data, 0, sizeof(flow_data.data));

    uint8_t *address_with_type_ptr;

    MUST_THROW(address_with_type_ptr =
                   get_output_address_ptr(flow_data.api, flow_data.read_index));

    // generate bech32 address including the address_type
    // since the struct is packed, the address follows directly the address_type
    address_encode_bech32(address_with_type_ptr, flow_data.data,
                          sizeof(flow_data.data));
}

static void cb_output_preinit()
{
    // clear buffer
    memset(flow_data.data, 0, sizeof(flow_data.data));

    switch (flow_data.type) {
    case REMAINDER:
        strcpy(flow_data.data, "Remainder");
        break;
    case OUTPUT: {
        // how many non-remainder outputs are there?
        // this is safe because the case of an essence with only one
        // remainder address as output is already covered
        // (is_bip32_remainder flag would be set).
        int non_remainder_outputs = flow_data.api->essence.outputs_count -
                                    !!flow_data.api->essence.has_remainder;

        // more than one? Show with numbers on the UI
        if (non_remainder_outputs > 1) {
            snprintf(flow_data.data, sizeof(flow_data.data) - 1, "Output [%d]",
                     flow_data.flow_outputs_index_current + 1);
        }
        else {
            strcpy(flow_data.data, "Output");
        }
        break;
    }
    default:
        THROW(SW_UNKNOWN);
        break;
    }
}

static void get_type_and_read_index()
{
    // default is normal output
    flow_data.type = OUTPUT;

    // translate the index of the data if needed
    // in user-confirm-mode reorder the datasets in a way that remainder always
    // is the last dateset
    flow_data.read_index = flow_data.flow_outputs_index_current;

    // does essence contain a remainder?
    if (flow_data.api->essence.has_remainder) {
        // is the remainder the last output in the essence?
        if (flow_data.api->essence.remainder_index ==
            flow_data.api->essence.outputs_count - 1) {
            // yes, but current index only is the remainder if it's the
            // remainder_index in case of an essence with only one remainder
            // output, this also would be true
            if (flow_data.read_index ==
                flow_data.api->essence.remainder_index) {
                flow_data.type = REMAINDER;
            }
        }
        else {
            // no it's not - we have to take care about switching indices
            // for displaying on the UI current read_index is the last
            // dataset? -> display remainder current read_index is the
            // remainder? -> display last dataset
            if (flow_data.read_index ==
                flow_data.api->essence.outputs_count - 1) {
                flow_data.read_index = flow_data.api->essence.remainder_index;
                flow_data.type = REMAINDER;
            }
            else if (flow_data.read_index ==
                     flow_data.api->essence.remainder_index) {
                flow_data.read_index = flow_data.api->essence.outputs_count - 1;
            }
        }
    }
}

// gets called by a timer to toggle the short and full view
static void cb_value_toggle()
{
    flow_data.amount_toggle = 1 - flow_data.amount_toggle;
    ux_flow_init(0, ux_flow_base, &ux_step_amount);
}


void flow_start_user_confirm_transaction(const API_CTX *api,
                                         accept_cb_t accept_cb,
                                         reject_cb_t reject_cb,
                                         timeout_cb_t timeout_cb)
{
    flow_start_user_confirm(api, accept_cb, reject_cb, timeout_cb);

    get_type_and_read_index();

    ux_flow_init(0, ux_flow_base, &ux_step_review);
}
