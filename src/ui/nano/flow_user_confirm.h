/*
 * flow.h
 *
 *  Created on: 21.10.2020
 *      Author: thomas
 */
#pragma once

#include <stdint.h>

#include "api.h"

// bech32: 62/63 bytes + 3 line breaks = 65/66
// essence hash: 66 bytes + 3 line breaks = 69
#define TMP_DATA_SIZE 80

// typedef unsigned int (*ux_callback_cb_t)(const bagl_element_t *e);
typedef void (*ux_fetch_data)();

typedef void (*accept_cb_t)();
typedef void (*reject_cb_t)();
typedef void (*timeout_cb_t)();

#define MUST_THROW(c)                                                          \
    {                                                                          \
        if (!(c)) {                                                            \
            THROW(SW_UNKNOWN);                                                 \
        }                                                                      \
    }


// struct that contains the data to be displayed on the screen
// during confirming the outputs
typedef struct {
    const API_CTX *api;

    // current index of output displayed
    short flow_outputs_index_current;

    // callbacks
    accept_cb_t accept_cb;
    reject_cb_t reject_cb;
    timeout_cb_t timeout_cb;

    int read_index;
    int num_non_remainder_outputs;

    // buffer for renderings of bech32 addresses, hashs, as temporary buffer,
    // ...
    char scratch[2][TMP_DATA_SIZE + 1]; // +1 zero terminator

    // total number of lines
    uint8_t number_of_lines;

    // toggles between short and full format of amount
    uint8_t amount_toggle;

    // for UI-100s-timeout
    uint32_t flow_timer_start;

    // flag that indicates the flow is active
    uint8_t flow_active;
} flowdata_t;

void flow_main_menu(void);

void flow_start_user_confirm(const API_CTX *api, accept_cb_t accept_cb,
                             reject_cb_t reject_cb, timeout_cb_t timeout_cb);


void flow_init(void);

void flow_stop(void);

void flow_timer_event(void);
