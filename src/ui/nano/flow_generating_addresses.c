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
    ux_generating_addresses,
    pbb,
    {
        &C_x_icon_load,
        "Generating",
        "Addresses ..."
    }
);

UX_FLOW(
    ux_flow_generating_addresses,
    &ux_generating_addresses,
    FLOW_END_STEP
);
// clang-format on

void flow_generating_addresses()
{
    ux_flow_init(0, ux_flow_generating_addresses, NULL);
}
