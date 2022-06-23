/*
 * flow_rejected.c
 *
 *  Created on: 26.10.2020
 *      Author: thomas
 */

#include "os.h"
#include "ux.h"
#include "glyphs.h"
#include "flow_main_menu.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

// clang-format off
UX_STEP_NOCB(
    ux_error_rejected,
    pb,
    {
        &C_x_icon_info,
        "Rejected"
    }
);

UX_FLOW(
    ux_flow_error_rejected,
    &ux_error_rejected,
    FLOW_END_STEP
);
// clang-format on

void flow_rejected()
{
    ux_flow_init(0, ux_flow_error_rejected, NULL);
}
