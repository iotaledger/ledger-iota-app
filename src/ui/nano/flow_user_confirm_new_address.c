#include "ux.h"
#include "glyphs.h"

#include "ui_common.h"
#include "flow_user_confirm.h"
#include "flow_user_confirm_new_address.h"
#include "abstraction.h"

extern flowdata_t flow_data;

// gcc doesn't know this and ledger's SDK cannot be compiled with Werror!
//#pragma GCC diagnostic error "-Werror"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

static void cb_address_preinit();

static void cb_accept();
static void cb_fix();
static void cb_fix2();

// clang-format off

static UX_STEP_NOCB_INIT(
    ux_step_new_address,
    bn_paging,
    cb_address_preinit(),
    {
        "Receive Address", (const char*) flow_data.scratch[0]
    }
);

static UX_STEP_NOCB_INIT(
    ux_step_new_remainder,
    bn_paging,
    cb_address_preinit(),
    {
        "New Remainder", (const char*) flow_data.scratch[0]
    }
);

static UX_STEP_CB(
    ux_step_ok,
    pb,
    cb_accept(),
    {
        &C_x_icon_check,
        "Ok"
    }
);

static UX_STEP_INIT(
    ux_step_fix,
    NULL,
    NULL,
    cb_fix()
);

static UX_STEP_INIT(
    ux_step_fix2,
    NULL,
    NULL,
    cb_fix2()
);

static UX_FLOW(
    ux_flow_new_address,
    &ux_step_fix2,
    &ux_step_new_address,
    &ux_step_ok,
    &ux_step_fix,
    FLOW_LOOP
);

static UX_FLOW(
    ux_flow_new_remainder,
    &ux_step_fix2,
    &ux_step_new_remainder,
    &ux_step_ok,
    &ux_step_fix,
    FLOW_LOOP
);

// clang-format on

static void cb_address_preinit()
{
    // clear buffer
    memset(flow_data.scratch[0], 0, sizeof(flow_data.scratch[0]));
    memset(flow_data.scratch[1], 0, sizeof(flow_data.scratch[1]));

    // generate bech32 address including the address_type
    // we only have a single address in the buffer starting at index 0
    address_encode_bech32(flow_data.api->data.buffer, flow_data.scratch[1],
                          sizeof(flow_data.scratch[1]));

    // insert line-breaks
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

static void cb_accept()
{
    if (flow_data.accept_cb) {
        flow_data.accept_cb();
    }
    flow_stop();
}

// fixes some weird paging issues (stepping forward, skips page 1/2)
static void cb_fix()
{
    if (flow_data.api->bip32_path[BIP32_CHANGE_INDEX] & 0x1) {
        ux_flow_init(0, ux_flow_new_remainder, &ux_step_new_remainder);
    }
    else {
        ux_flow_init(0, ux_flow_new_address, &ux_step_new_remainder);
    }
}

// fixes some weird paging issues (stepping forward, skips page 1/2)
static void cb_fix2()
{
    if (flow_data.api->bip32_path[BIP32_CHANGE_INDEX] & 0x1) {
        ux_flow_init(0, ux_flow_new_remainder, &ux_step_ok);
    }
    else {
        ux_flow_init(0, ux_flow_new_address, &ux_step_ok);
    }
}

void flow_start_new_address(const API_CTX *api, accept_cb_t accept_cb,
                            timeout_cb_t timeout_cb)
{
    flow_start_user_confirm(api, accept_cb, 0, timeout_cb);

    if (flow_data.api->bip32_path[BIP32_CHANGE_INDEX] & 0x1) {
        ux_flow_init(0, ux_flow_new_remainder, &ux_step_new_remainder);
    }
    else {
        ux_flow_init(0, ux_flow_new_address, &ux_step_new_address);
    }
}
