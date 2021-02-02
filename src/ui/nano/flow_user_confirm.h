/*
 * flow.h
 *
 *  Created on: 21.10.2020
 *      Author: thomas
 */

#ifndef SRC_UI_NANO_FLOW_USER_CONFIRM_H_
#define SRC_UI_NANO_FLOW_USER_CONFIRM_H_

#include <stdint.h>

#include "api.h"

typedef void (*accept_cb_t)();
typedef void (*reject_cb_t)();
typedef void (*timeout_cb_t)();


void flow_main_menu();

void flow_init();

void flow_start_user_confirm(const API_CTX *api, accept_cb_t accept_cb, reject_cb_t reject_cb, timeout_cb_t timeout_cb, const uint32_t bip32[BIP32_PATH_LEN]);
void flow_start_new_address(const API_CTX *api, accept_cb_t accept_cb, timeout_cb_t timeout_cb, const uint32_t bip32[BIP32_PATH_LEN]);

void flow_stop();

void flow_timer_event();

#endif /* SRC_UI_NANO_FLOW_USER_CONFIRM_H_ */
