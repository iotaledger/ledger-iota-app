#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "ui_common.h"
#include "flow_user_confirm.h"
#include "flow_user_confirm_new_address.h"
#include "abstraction.h"

extern flowdata_t flow_data;

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

static void cb_address_preinit();
static void cb_bip32_preinit();

static void cb_accept();

static void cb_na_fix();
static void cb_na_fix2();

// clang-format off
UX_STEP_NOCB_INIT(
    ux_step_new_address,
    bn_paging,
    cb_address_preinit(),
    {
        // in paging mode, "New Remainder" doesn't fit without 
        // wrapping in the next line
        (const char*) flow_data.scratch[1], (const char*) flow_data.scratch[0]
    }
);

#ifdef TARGET_NANOS    
UX_STEP_NOCB_INIT(
    ux_step_na_bip32,
    bn_paging,
    cb_bip32_preinit(),
    {
        "BIP32 Path", (const char*) flow_data.scratch[0]
    }
);
#else
UX_STEP_NOCB_INIT(
    ux_step_na_bip32,
    bn,
    cb_bip32_preinit(),
    {
        "BIP32 Path", (const char*) flow_data.scratch[0]
    }
);
#endif

UX_STEP_CB(
    ux_step_ok,
    pb,
    cb_accept(),
    {
        &C_x_icon_check,
        "Ok"
    }
);

UX_STEP_INIT(
    ux_step_na_fix,
    NULL,
    NULL,
    cb_na_fix()
);

UX_STEP_INIT(
    ux_step_na_fix2,
    NULL,
    NULL,
    cb_na_fix2()
);

UX_FLOW(
    ux_flow_new_address,
    &ux_step_na_fix2,
    &ux_step_new_address,
    &ux_step_na_bip32,
    &ux_step_ok,
    &ux_step_na_fix,
    FLOW_LOOP
);

// clang-format on

static void cb_address_preinit()
{
    // clear buffer
    memset(flow_data.scratch[0], 0, sizeof(flow_data.scratch[0]));
    memset(flow_data.scratch[1], 0, sizeof(flow_data.scratch[1]));

    // header
    if (flow_data.api->bip32_path[BIP32_CHANGE_INDEX] & 0x1) {
#ifdef TARGET_NANOS
        strncpy(flow_data.scratch[1], "Remainder",
                sizeof(flow_data.scratch[1]) - 1);
#else
        strncpy(flow_data.scratch[1], "New Remainder",
                sizeof(flow_data.scratch[1]) - 1);
#endif
    }
    else {
#ifdef TARGET_NANOS
        strncpy(flow_data.scratch[1], "Address",
                sizeof(flow_data.scratch[1]) - 1);
#else
        strncpy(flow_data.scratch[1], "Receive Address",
                sizeof(flow_data.scratch[1]) - 1);
#endif
    }

    // generate bech32 address including the address_type
    // we only have a single address in the buffer starting at index 0
    address_encode_bech32(flow_data.api->data.buffer, flow_data.scratch[0],
                          sizeof(flow_data.scratch[0]));
}

static void cb_bip32_preinit()
{
    // clear buffer
    memset(flow_data.scratch[0], 0, sizeof(flow_data.scratch[0]));

    format_bip32_with_line_breaks(flow_data.api->bip32_path,
                                  flow_data.scratch[0],
                                  sizeof(flow_data.scratch[0]));
}

static void cb_accept()
{
    if (flow_data.accept_cb) {
        flow_data.accept_cb();
    }
    flow_stop();
}

// fixes some weird paging issues (stepping forward, skips pages)
static void cb_na_fix()
{
    ux_flow_init(0, ux_flow_new_address, &ux_step_new_address);
}

// fixes some weird paging issues (stepping forward, skips pages)
static void cb_na_fix2()
{
    ux_flow_init(0, ux_flow_new_address, &ux_step_ok);
}

void flow_start_new_address(const API_CTX *api, accept_cb_t accept_cb,
                            timeout_cb_t timeout_cb)
{
    flow_start_user_confirm(api, accept_cb, 0, timeout_cb);

    ux_flow_init(0, ux_flow_new_address, &ux_step_new_address);
}
