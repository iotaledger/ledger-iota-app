/*
 * flow_main_menu.c
 *
 *  Created on: 26.10.2020
 *      Author: thomas
 */
#include "flow_user_confirm.h"

#include "ux.h"
#include "glyphs.h"

#include "api.h"

#include "iota/constants.h"
#include "ui_common.h"

// gcc doesn't know this and ledger's SDK cannot be compiled with Werror!
//#pragma GCC diagnostic error "-Werror"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

// clang-format off
UX_STEP_NOCB(
    ux_idle_flow_1_step, pb,
    {
        &C_icon_iota,
        "IOTA",
    }
);

UX_STEP_NOCB(
    ux_idle_flow_2_step,
    bn,
    {
    "Version",
    APPVERSION,
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
    &ux_idle_flow_3_step,
    FLOW_LOOP
);
// clang-format on

void flow_main_menu() {
	// reserve a display stack slot if none yet
	if (G_ux.stack_count == 0) {
		ux_stack_push();
	}
	ux_flow_init(0, ux_main_menu, NULL);
}
