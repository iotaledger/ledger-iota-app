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


static unsigned int ux_error_generic_cb(const bagl_element_t *e)
{
    UNUSED(e);
    return 0;
}

// clang-format off
UX_STEP_CB(
    ux_error_generic,
    pb,
    ux_error_generic_cb(NULL),
    {
        &C_x_icon_info,
        "Error"
    }
);

UX_FLOW(
    ux_flow_error_generic,
    &ux_error_generic,
    FLOW_END_STEP
);
// clang-format off

void flow_generic_error()
{
    ux_flow_init(0, ux_flow_error_generic, NULL);
}
