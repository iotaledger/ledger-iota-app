/*
 * address.h
 *
 *  Created on: 16.10.2020
 *      Author: thomas
 */

#pragma once

#include <stdint.h>

uint8_t address_encode_bech32_hrp(const uint8_t *addr, char *bech32,
                                  uint32_t bech32_max_length, const char *hrp,
                                  const size_t hrp_len);

uint8_t address_generate(uint32_t *bip32_path, uint32_t bip32_path_length,
                         uint8_t *addr);
