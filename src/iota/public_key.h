/*
 * address.h
 *
 *  Created on: 16.10.2020
 *      Author: thomas
 */

#pragma once

#include <stdint.h>

uint8_t public_key_generate(uint32_t *bip32_path, uint32_t bip32_path_length,
                            uint8_t *addr);
