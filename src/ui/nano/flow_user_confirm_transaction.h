/*
 * flow.h
 *
 *  Created on: 21.10.2020
 *      Author: thomas
 */
#pragma once

#include <stdint.h>

#include "api.h"

#include "flow_user_confirm.h"

void flow_start_user_confirm_transaction(const API_CTX *api,
                                         accept_cb_t accept_cb,
                                         reject_cb_t reject_cb,
                                         timeout_cb_t timeout_cb);
