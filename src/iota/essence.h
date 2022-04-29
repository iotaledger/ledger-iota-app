/*
 * essence.h
 *
 *  Created on: 16.10.2020
 *      Author: thomas
 */

#ifndef SRC_IOTA_ESSENCE_H_
#define SRC_IOTA_ESSENCE_H_

#include <stdint.h>
#include "api.h"

uint8_t essence_parse_and_validate(API_CTX *api);

uint8_t essence_parse_and_validate_blindsigning(API_CTX *api);

#endif /* SRC_IOTA_ESSENCE_H_ */
