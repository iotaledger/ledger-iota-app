#include "ui_common.h"

#include "ux.h"
#include "glyphs.h"

#include "flow_user_confirm.h"
#include "flow_user_confirm_blindsigning.h"

// gcc doesn't know this and ledger's SDK cannot be compiled with Werror!
//#pragma GCC diagnostic error "-Werror"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

extern flowdata_t flow_data;

static void cb_hash_preinit();

static void cb_bs_accept();
static void cb_bs_reject();

static void cb_hash_preinit();

// clang-format off

static UX_STEP_NOCB_INIT(
    ux_step_hash,
    bn_paging,
    cb_hash_preinit(),
    {
        "Blind Signing", (const char*) flow_data.scratch[0]
    }
);

static UX_STEP_CB(
    ux_step_bs_accept,
    pb,
    cb_bs_accept(NULL),
    {
        &C_x_icon_check,
        "Accept"
    }
);

static UX_STEP_CB(
    ux_step_bs_reject,
    pb,
    cb_bs_reject(NULL),
    {
        &C_x_icon_cross,
        "Reject"
    }
);

static UX_FLOW(
    ux_blindsigning,
    &ux_step_hash,
    &ux_step_bs_accept,
    &ux_step_bs_reject,
    FLOW_LOOP
);

// clang-format on


void cb_hash_preinit()
{
    const char *hex = "0123456789ABCDEF";

    // generate hash
    memset(flow_data.scratch[0], 0, sizeof(flow_data.scratch[0]));

    const char *src = (const char *)flow_data.api->essence.hash;
    char *dst = flow_data.scratch[0];

    *dst++ = '0';
    *dst++ = 'x';
    for (uint8_t i = 0; i < 32; i++) {
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

    ux_flow_init(0, ux_blindsigning, &ux_step_hash);
}
