#pragma once

#include "flow_user_confirm.h"

void flow_start_new_address(const API_CTX *api, accept_cb_t accept_cb,
                            timeout_cb_t timeout_cb);
