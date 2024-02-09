/*
 * generic_error.c
 *
 *  Created on: 26.10.2020
 *      Author: thomas
 */

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "os_io_seproxyhal.h"

#include "flow_main_menu.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

extern const ux_flow_step_t *const ux_main_menu;

// clang-format off
UX_STEP_TIMEOUT(
    ux_generating_public_keys,
    pbb,
    1000,    // show main menu after 1s
    &ux_main_menu,
    {
        &C_x_icon_load,
        "Generating",
        "Public Keys ..."
    }
);

UX_FLOW(
    ux_flow_generating_public_keys,
    &ux_generating_public_keys,
    FLOW_END_STEP
);
// clang-format on

void flow_generating_public_keys()
{
    ux_flow_init(0, ux_flow_generating_public_keys, NULL);

    // show flow immediately
    UX_WAIT_DISPLAYED();
}
