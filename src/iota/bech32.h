/*
 * bech32.h
 *
 *  Created on: 16.10.2020
 *      Author: thomas
 */

#ifndef SRC_IOTA_BECH32_H_
#define SRC_IOTA_BECH32_H_

#include <stdint.h>
#include <stddef.h>

int bech32_encode(char *const output, size_t *const out_len,
                  const char *const hrp, const size_t hrp_len,
                  const uint8_t *const data, const size_t data_len);

int base32_encode(uint8_t *const out, size_t *out_len, const uint8_t *const in,
                  const size_t in_len);


#endif /* SRC_IOTA_BECH32_H_ */
