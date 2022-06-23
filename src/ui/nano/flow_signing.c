/*
 * generic_error.c
 *
 *  Created on: 26.10.2020
 *      Author: thomas
 */

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

// clang-format off
UX_STEP_NOCB(
    ux_signing,
    pb,
    {
        &C_x_icon_load,
        "Signing ..."
    }
);

UX_FLOW(
    ux_flow_signing,
    &ux_signing,
    FLOW_END_STEP
);
// clang-format on

void flow_signing()
{
    ux_flow_init(0, ux_flow_signing, NULL);
}
