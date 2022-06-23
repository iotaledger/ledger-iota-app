/*
 * flow.c
 *
 *  Created on: 21.10.2020
 *      Author: thomas
 */

#include "os.h"
#include "flow_user_confirm_transaction.h"
#include "abstraction.h"
#include "ux.h"

#include "glyphs.h"

#include "api.h"

#include "iota/constants.h"
#include "ui_common.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

flowdata_t flow_data;

extern uint32_t timer_events;

void flow_init()
{
    memset(&flow_data, 0, sizeof(flow_data));
}

void flow_stop()
{
    flow_init();
    flow_main_menu();
}

void flow_start_user_confirm(const API_CTX *api, accept_cb_t accept_cb,
                             reject_cb_t reject_cb, timeout_cb_t timeout_cb)
{
    flow_init();

    flow_data.api = api;

    flow_data.accept_cb = accept_cb;
    flow_data.reject_cb = reject_cb;
    flow_data.timeout_cb = timeout_cb;

    flow_data.flow_timer_start = timer_events;

    flow_data.flow_active = 1;

    flow_data.amount_toggle = 0;
}

void flow_timer_event()
{
    if (flow_data.flow_active) {
        if ((timer_events - flow_data.flow_timer_start) >= 1000 /* 100s */) {
            if (flow_data.timeout_cb) {
                flow_data.timeout_cb();
            }
            flow_stop();
        }
    }
}
