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
    ux_signed_successfully,
    pbb,
    2000,
    &ux_main_menu,
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

    // show flow immediately
    UX_WAIT_DISPLAYED();
}
