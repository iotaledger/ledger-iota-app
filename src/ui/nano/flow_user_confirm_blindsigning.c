#include "os.h"
#include "ui_common.h"

#include "ux.h"
#include "glyphs.h"

#include "nv_mem.h"

#include "flow_user_confirm.h"
#include "flow_user_confirm_blindsigning.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

extern flowdata_t flow_data;

static void cb_hash_preinit();

static void cb_bs_accept();
static void cb_bs_reject();

static void cb_hash_preinit();

// clang-format off

UX_STEP_NOCB_INIT(
    ux_step_hash,
    bn_paging,
    cb_hash_preinit(),
    {
        "Blind Signing", (const char*) flow_data.scratch[0]
    }
);

UX_STEP_CB(
    ux_step_bs_accept,
    pb,
    cb_bs_accept(),
    {
        &C_x_icon_check,
        "Accept"
    }
);

UX_STEP_CB(
    ux_step_bs_reject,
    pb,
    cb_bs_reject(),
    {
        &C_x_icon_cross,
        "Reject"
    }
);


UX_FLOW(
    ux_flow_blindsigning,
    &ux_step_hash,
    &ux_step_bs_accept,
    &ux_step_bs_reject,
    FLOW_LOOP
);

UX_STEP_CB(
    ux_step_bs_not_enabled,
    pbb,
    cb_bs_reject(),
    {
        &C_x_icon_cross, "Blind Signing", "not enabled!"
    }
);



UX_FLOW(
    ux_flow_bs_not_enabled,
    &ux_step_bs_not_enabled,
    &ux_step_bs_reject,
    FLOW_LOOP
);


// clang-format on


void cb_hash_preinit()
{
    const char *hex = "0123456789ABCDEF";

    // generate hash
    memset(flow_data.scratch[0], 0, sizeof(flow_data.scratch[0]));

    const char *src = (const char *)flow_data.api->essence.signing_input;
    char *dst = flow_data.scratch[0];

    uint8_t len = flow_data.api->essence.signing_input_len;

    *dst++ = '0';
    *dst++ = 'x';
    for (uint8_t i = 0; i < len; i++) {
        *dst++ = hex[*src >> 4];
        *dst++ = hex[*src++ & 0x0f];
    }
    *dst = 0;
}

static void cb_bs_accept()
{
    if (flow_data.accept_cb) {
        flow_data.accept_cb();
    }
    flow_stop();
}

static void cb_bs_reject()
{
    if (flow_data.reject_cb) {
        flow_data.reject_cb();
    }
    flow_stop();
}

void flow_start_blindsigning(const API_CTX *api, accept_cb_t accept_cb,
                             reject_cb_t reject_cb, timeout_cb_t timeout_cb)
{
    flow_start_user_confirm(api, accept_cb, reject_cb, timeout_cb);

    if (!nv_get_blindsigning()) {
        ux_flow_init(0, ux_flow_bs_not_enabled, &ux_step_bs_not_enabled);
    }
    else {
        ux_flow_init(0, ux_flow_blindsigning, &ux_step_hash);
    }
}
