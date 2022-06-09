/*
 * bech32.h
 *
 *  Created on: 16.10.2020
 *      Author: thomas
 */

#pragma once

#include <stdint.h>

int bech32_encode(char *const output, size_t *const out_len,
                  const char *const hrp, const size_t hrp_len,
                  const uint8_t *const data, const size_t data_len);

int base32_encode(uint8_t *const out, size_t *out_len, const uint8_t *const in,
                  const size_t in_len);
