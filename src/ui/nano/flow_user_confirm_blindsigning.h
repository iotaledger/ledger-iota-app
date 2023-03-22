#pragma once

#include "api.h"

void flow_start_blindsigning(const API_CTX *api, accept_cb_t accept_cb,
                             reject_cb_t reject_cb, timeout_cb_t timeout_cb);