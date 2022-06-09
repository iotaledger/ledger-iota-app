#pragma once

#include <stdint.h>

#include "api.h"

const uint8_t *get_output_address_ptr(const API_CTX *api, uint8_t index);

uint64_t get_output_amount(const API_CTX *api, uint8_t index);

uint8_t address_encode_bech32(const uint8_t *addr_with_type, char *bech32,
                              uint32_t bech32_max_length);

uint8_t essence_parse_and_validate(API_CTX *api);

uint8_t get_amount(const API_CTX *api, int index, char *dst, size_t dst_len,
                   uint8_t full);
