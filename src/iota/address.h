/*
 * address.h
 *
 *  Created on: 16.10.2020
 *      Author: thomas
 */

#ifndef SRC_IOTA_ADDRESS_H_
#define SRC_IOTA_ADDRESS_H_

#include "api.h"
#include "constants.h"

uint8_t address_encode_bech32(const uint8_t* addr, char* bech32, uint32_t bech32_max_length);

uint8_t address_generate(uint32_t* bip32_path, uint32_t bip32_path_length, uint8_t* addr);


#endif /* SRC_IOTA_ADDRESS_H_ */
