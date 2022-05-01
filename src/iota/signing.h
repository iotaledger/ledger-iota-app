#pragma once

#include "api.h"

uint16_t sign(API_CTX *api, uint8_t *output,
                             uint16_t output_max_len, uint32_t signature_index);
