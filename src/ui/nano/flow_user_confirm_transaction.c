/*
 * flow.c
 *
 *  Created on: 21.10.2020
 *      Author: thomas
 */

#include "os.h"
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


/**
 * Define a flow step with autovalidation after a given timeout (in ms)
 * with additional pre-init. Used for toggeling the IOTA amounts between short
 * and full format
 */
#define UX_STEP_TIMEOUT_INIT(stepname, layoutkind, preinit, timeout_ms,        \
                             validate_flow, ...)                               \
    void stepname##_init(unsigned int stack_slot)                              \
    {                                                                          \
        preinit;                                                               \
        ux_layout_##layoutkind##_init(stack_slot);                             \
        ux_layout_set_timeout(stack_slot, timeout_ms);                         \
    }                                                                          \
    const ux_layout_##layoutkind##_params_t stepname##_val = __VA_ARGS__;      \
    const ux_flow_step_t stepname = {                                          \
        stepname##_init,                                                       \
        &stepname##_val,                                                       \
        validate_flow,                                                         \
        NULL,                                                                  \
    }


extern flowdata_t flow_data;

static void cb_value_toggle();

static void cb_accept();
static void cb_reject();
static void cb_continue_claiming();

static void cb_address_preinit();
static void cb_amount_preinit();
static void cb_output_preinit();
static void cb_bip32_preinit();

static void cb_switch();
static void cb_back();

static void cb_next_dataset();
static void cb_prev_dataset();

static void ui_confirm_output_value_toggle();

static void get_type_and_read_index();


//------------------------------------------------------------------------
// clang-format off

UX_FLOW_CALL(
        ux_flow_datasets_value_toggle,
        cb_value_toggle()
)

// review output [...]
UX_STEP_NOCB_INIT(
    ux_step_review,
    bb,
    cb_output_preinit(),
    {
        "Review", (const char*) flow_data.scratch[0]
    }
);

UX_FLOW_CALL(
        ux_flow_confirm_output_value_toggle,
        ui_confirm_output_value_toggle()
)

UX_STEP_TIMEOUT_INIT(
    ux_step_amount,
    bn,
    cb_amount_preinit(),
    2500,
    ux_flow_confirm_output_value_toggle,
    {
        (const char*) flow_data.scratch[1], (const char*) flow_data.scratch[0]

    }
);

UX_STEP_NOCB_INIT(
    ux_step_address,
    bn_paging,
    cb_address_preinit(),
    {
        "Address", (const char*) flow_data.scratch[0]
    }
);

#ifdef TARGET_NANOS    
UX_STEP_NOCB_INIT(
    ux_step_remainder,
    bn_paging,
    cb_bip32_preinit(),
    {
        "BIP32 Path", (const char*) flow_data.scratch[0]
    }
);
#else
UX_STEP_NOCB_INIT(
    ux_step_remainder,
    bn,
    cb_bip32_preinit(),
    {
        "BIP32 Path", (const char*) flow_data.scratch[0]
    }
);
#endif

UX_STEP_INIT(
    ux_step_switch,
    NULL,
    NULL,
    cb_switch()
);

UX_STEP_INIT(
    ux_step_data_next,
    NULL,
    NULL,
    cb_next_dataset()
);

UX_STEP_INIT(
    ux_step_data_prev,
    NULL,
    NULL,
    cb_prev_dataset()
);

UX_STEP_INIT(
    ux_step_back,
    NULL,
    NULL,
    cb_back()
);
//------------------------------------------------------------------------

UX_STEP_CB(
    ux_step_accept,
    pb,
    cb_accept(NULL),
    {
        &C_x_icon_check,
        "Accept"
    }
);

UX_STEP_CB(
    ux_step_reject,
    pb,
    cb_reject(NULL),
    {
        &C_x_icon_cross,
        "Reject"
    }
);

UX_FLOW(
    ux_flow_base,
    &ux_step_data_prev,
    &ux_step_review,
    &ux_step_address,
    &ux_step_amount,
    &ux_step_switch,
    &ux_step_data_next,
    FLOW_LOOP
);

UX_FLOW(
    ux_flow_has_remainder,
    &ux_step_back,
    &ux_step_remainder,
    &ux_step_data_next,
    FLOW_LOOP
);

UX_FLOW(
    ux_flow_has_accept_reject,
    &ux_step_back,
    &ux_step_accept,
    &ux_step_reject,
    &ux_step_data_next,
    FLOW_LOOP
);

UX_FLOW(
    ux_flow_has_remainder_accept_reject,
    &ux_step_back,
    &ux_step_remainder,
    &ux_step_accept,
    &ux_step_reject,
    &ux_step_data_next,
    FLOW_LOOP
);

//--------------------------------------------
// SMR claiming

UX_STEP_NOCB(
    ux_step_srm_claiming_start,
    pb,
    {
        &C_icon_warning,
        "Claim SMR"
    }
);

UX_STEP_NOCB_INIT(
    ux_step_smr_claiming_message,
    bn_paging,
    cb_address_preinit(),
    {
        "Claim SMR", "In order to claim the SMR token, you are now "
                     "signing with IOTA private keys instead of SMR "
                     "private keys. "
                     "Are you really sure you want to proceed?"
    }
);

UX_STEP_CB(
    ux_step_continue,
    pb,
    cb_continue_claiming(NULL),
    {
        &C_x_icon_check,
        "Continue"
    }
);

UX_STEP_CB(
    ux_step_cancel,
    pb,
    cb_reject(NULL),
    {
        &C_x_icon_cross,
        "Cancel"
    }
);

UX_FLOW(
    ux_flow_smr_claiming_start,
    &ux_step_srm_claiming_start,
    &ux_step_smr_claiming_message,
    &ux_step_continue,
    &ux_step_cancel,
    FLOW_LOOP
);

//--------------------------------------------
// Sweeping Transaction

UX_STEP_NOCB(
    ux_step_sweeping_start,
    pbb,
    {
        &C_x_icon_info,
        "Internal",
        "Transfer"
    }
);

UX_STEP_NOCB_INIT(
    ux_step_sweeping_info,
    nn,
    cb_address_preinit(),
    {
        "Info: All coins ", "remain on the wallet"
    }
);

UX_FLOW(
    ux_flow_sweeping,
    &ux_step_sweeping_start,
    &ux_step_sweeping_info,
    &ux_step_accept,
    &ux_step_reject,
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

    // reset toggle flag
    flow_data.amount_toggle = 0;

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

    // reset toggle flag
    flow_data.amount_toggle = 0;

    uint8_t remainder = !!(flow_data.type == REMAINDER);
    uint8_t last = !!(flow_data.flow_outputs_index_current ==
                      flow_data.api->essence.outputs_count - 1);

    uint8_t m = (last << 1) | remainder;
    ux_flow_init(0, jump_table_prev[m].flow, jump_table_prev[m].step);
}

// gets called by a timer to toggle the short and full view
static void ui_confirm_output_value_toggle()
{
    flow_data.amount_toggle = 1 - flow_data.amount_toggle;
    ux_flow_init(0, ux_flow_base, &ux_step_amount);
}

static void cb_back()
{
    ux_flow_init(0, ux_flow_base, &ux_step_amount);
}

static void cb_accept()
{
    if (flow_data.accept_cb) {
        flow_data.accept_cb();
    }
    flow_stop();
}

static void cb_reject()
{
    if (flow_data.reject_cb) {
        flow_data.reject_cb();
    }
    flow_stop();
}

static void cb_continue_claiming()
{
    // user acknodlwedged to continue
    // now start the actual transaction confirming flow
    ux_flow_init(0, ux_flow_base, &ux_step_review);
}

static void cb_amount_preinit()
{
    // clear buffer
    memset(flow_data.scratch[0], 0, sizeof(flow_data.scratch[0]));

    MUST_THROW(get_amount(flow_data.api, flow_data.read_index,
                          flow_data.scratch[0], sizeof(flow_data.scratch[0]),
                          flow_data.amount_toggle));

    // copy header after writing amount
    if (flow_data.api->coin == COIN_SHIMMER) {
        strncpy(flow_data.scratch[1], "Amount SMR",
                sizeof(flow_data.scratch[1]));
    }
    else {
        strncpy(flow_data.scratch[1], "Amount IOTA",
                sizeof(flow_data.scratch[1]));
    }
}

static void cb_bip32_preinit()
{
    // clear buffer
    memset(flow_data.scratch[0], 0, sizeof(flow_data.scratch[0]));

    format_bip32_with_line_breaks(flow_data.api->bip32_path,
                                  flow_data.scratch[0],
                                  sizeof(flow_data.scratch[0]));
}

static void cb_address_preinit()
{
    // clear buffer
    memset(flow_data.scratch[0], 0, sizeof(flow_data.scratch[0]));
    memset(flow_data.scratch[1], 0, sizeof(flow_data.scratch[1]));

    const uint8_t *address_with_type_ptr = 0;

    MUST_THROW(address_with_type_ptr =
                   get_output_address_ptr(flow_data.api, flow_data.read_index));


    // generate bech32 address including the address_type
    // since the struct is packed, the address follows directly the address_type
    address_encode_bech32(address_with_type_ptr, flow_data.scratch[1],
                          sizeof(flow_data.scratch[1]));

    // insert max 3 line-breaks
    MUST_THROW(string_insert_chars_each(
        flow_data.scratch[1], sizeof(flow_data.scratch[1]),
        flow_data.scratch[0], sizeof(flow_data.scratch[0]), 16, 3, '\n'));

#ifdef TARGET_NANOS
    // NOP - paging of nanos is fine
#else
    memcpy(flow_data.scratch[1], flow_data.scratch[0],
           sizeof(flow_data.scratch[1]));
    // insert another line-break (2 lines per page)
    MUST_THROW(string_insert_chars_each(
        flow_data.scratch[1], sizeof(flow_data.scratch[1]),
        flow_data.scratch[0], sizeof(flow_data.scratch[0]), 33, 1, '\n'));
#endif
}

static void cb_output_preinit()
{
    // clear buffer
    memset(flow_data.scratch[0], 0, sizeof(flow_data.scratch[0]));
    memset(flow_data.scratch[1], 0, sizeof(flow_data.scratch[1]));

    if (flow_data.api->app_mode == APP_MODE_SHIMMER_CLAIMING) {
        strcpy(flow_data.scratch[1], "Claim SMR");
    } else {
        strcpy(flow_data.scratch[1], "Review");
    }

    switch (flow_data.type) {
    case REMAINDER:
        strcpy(flow_data.scratch[1], "Remainder");
        strcpy(flow_data.scratch[0], "Output");
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
            snprintf(flow_data.scratch[0], sizeof(flow_data.scratch[0]) - 1,
                     "Output [%d]", flow_data.flow_outputs_index_current + 1);
        }
        else {
            strcpy(flow_data.scratch[0], "Output");
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
    
    if (api->app_mode == APP_MODE_SHIMMER_CLAIMING) {
        // show claiming smr message before starting regular flow
        ux_flow_init(0, ux_flow_smr_claiming_start, &ux_step_srm_claiming_start);
        return;
    }
    
    if (api->essence.is_internal_transfer) {
        // if it's a different flow, we only show some info
        // there is no security risk because coins remain on the wallet
        ux_flow_init(0, ux_flow_sweeping, &ux_step_sweeping_start);
        return;
    }

    // start regular flow
    ux_flow_init(0, ux_flow_base, &ux_step_review);
}
