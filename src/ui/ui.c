
#include "ui.h"

#include "nano/flow_user_confirm.h"
#include "nano/flow_main_menu.h"
#include "nano/flow_generating_addresses.h"
#include "nano/flow_generic_error.h"
#include "nano/flow_rejected.h"
#include "nano/flow_signed_successfully.h"
#include "nano/flow_signing.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

void ui_reset()
{
}

void ui_init()
{
    flow_init();
    flow_main_menu();
}

void ui_timer_event()
{
    flow_timer_event();
}

uint8_t ui_show(uint8_t flow)
{
    switch (flow) {
    case FLOW_MAIN_MENU:
        flow_main_menu();
        break;
    case FLOW_GENERATING_ADDRESSES:
        flow_generating_addresses();
        break;
    case FLOW_GENERIC_ERROR:
        flow_generic_error();
        break;
    case FLOW_REJECTED:
        flow_rejected();
        break;
    case FLOW_SIGNED_SUCCESSFULLY:
        flow_signed_successfully();
        break;
    case FLOW_SIGNING:
        flow_signing();
        break;
    default:
        flow_main_menu();
        return 0;
    }
    return 1;
}
