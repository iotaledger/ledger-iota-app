#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "api.h"
#include "constants.h"

#define IO_STRUCT struct __attribute__((packed, may_alias))

#define MUST(c)                                                                \
    {                                                                          \
        if (!(c)) {                                                            \
            return 0;                                                          \
        }                                                                      \
    }


typedef IO_STRUCT
{
    // amount first for better data alignment
    // we can keep the struct packed but amount and address_type are
    // aligned on 32bits
    uint64_t amount;
    uint8_t address_type;
    uint8_t address[ADDRESS_SIZE_BYTES];
}
ABS_OUTPUT;

uint8_t *get_output_address_ptr(const API_CTX *api, uint8_t index);

uint64_t get_output_amount(const API_CTX *api, uint8_t index);

uint8_t address_encode_bech32(const uint8_t *addr_with_type, char *bech32,
                              uint32_t bech32_max_length);

uint8_t essence_parse_and_validate(API_CTX *api);
