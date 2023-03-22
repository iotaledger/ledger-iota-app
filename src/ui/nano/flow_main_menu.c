/*
 * flow_main_menu.c
 *
 *  Created on: 26.10.2020
 *      Author: thomas
 */
#include "os.h"
#include "ux.h"

#include "flow_user_confirm.h"

#include "glyphs.h"

#include "api.h"
#include "nv_mem.h"

#include "iota/constants.h"
#include "ui_common.h"

// enabled
// disabled

static char switch_state[9];

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

void cb_settings_preinit();
void cb_settings_enter();
void cb_settings_back();
void cb_toggle_blindsigning();

// clang-format off
#if defined(APP_IOTA)
UX_STEP_NOCB(
    ux_idle_flow_1_step, 
    pb,
    {
        &C_icon_iota,
        "IOTA",
    }
);
#elif defined(APP_SHIMMER)
UX_STEP_NOCB(
    ux_idle_flow_1_step, 
    pb,
    {
        &C_icon_shimmer,
        "Shimmer",
    }
);
#else
#error unknown app
#endif

UX_STEP_NOCB(
    ux_idle_flow_2_step,
    bn,
    {
        "Version",
        APPVERSION,
    }
);

UX_STEP_CB(
    ux_settings,
    pb,
    cb_settings_enter(),
    {
        &C_icon_coggle, "Settings",
    }    
);

UX_STEP_CB(
    ux_idle_flow_3_step,
    pb,
    os_sched_exit(-1),
    {
        &C_x_icon_dash, "Quit",
    }
);

UX_FLOW(ux_main_menu,
    &ux_idle_flow_1_step,
    &ux_idle_flow_2_step,
    &ux_settings,
    &ux_idle_flow_3_step,
    FLOW_LOOP
);

// settings menu


UX_STEP_CB_INIT(
    ux_step_toggle_blindsigning,
    bn,
    cb_settings_preinit(),
    cb_toggle_blindsigning(),
    {
        "Blind Signing",
        (const char*) switch_state
    }    
);

UX_STEP_CB(
    ux_step_settings_back,
    pb,
    cb_settings_back(),
    {
        &C_x_icon_back, "Back"
    }    
);

UX_FLOW(
    ux_flow_settings,
    &ux_step_toggle_blindsigning,
    &ux_step_settings_back,
    FLOW_LOOP
);

// clang-format on

void cb_settings_enter()
{
    ux_flow_init(0, ux_flow_settings, &ux_step_toggle_blindsigning);
}

void cb_settings_preinit()
{
    if (nv_get_blindsigning()) {
        strcpy(switch_state, "enabled");
    }
    else {
        strcpy(switch_state, "disabled");
    }
}

void cb_toggle_blindsigning()
{
    nv_toggle_blindsigning();
    ux_flow_init(0, ux_flow_settings, &ux_step_toggle_blindsigning);
}

void cb_settings_back()
{
    ux_flow_init(0, ux_main_menu, &ux_settings);
}

void flow_main_menu()
{
    // reserve a display stack slot if none yet
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_main_menu, NULL);
}
