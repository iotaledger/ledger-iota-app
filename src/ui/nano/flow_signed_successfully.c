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
    ux_signed_successfully,
    pbb,
    {
    &C_x_icon_check,
    "Signed",
    "Successfully"
    }
);

UX_FLOW(
    ux_flow_signed_successfully,
    &ux_signed_successfully,
    FLOW_END_STEP
);
// clang-format on

void flow_signed_successfully()
{
    ux_flow_init(0, ux_flow_signed_successfully, NULL);
}
