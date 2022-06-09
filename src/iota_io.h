#ifndef IOTA_IO_H
#define IOTA_IO_H

#include <stdbool.h>
#include <stdint.h>

#include "os.h"

#include "iota/constants.h"

void io_initialize(void);
void io_send(const void *ptr, unsigned int length, unsigned short sw);

uint8_t *io_get_buffer(void);


unsigned int iota_dispatch(uint8_t ins, uint8_t p1, uint8_t p2, uint8_t len,
                           const unsigned char *input_data, uint8_t is_locked);


/* ---  CLA  --- */

#define CLA 0x7B

/* ---  INS  --- */

enum {
    INS_NONE = 0x00,

    INS_GET_APP_CONFIG = 0x10,
    INS_SET_ACCOUNT = 0x11,

    INS_GET_DATA_BUFFER_STATE = 0x80,
    INS_WRITE_DATA_BLOCK = 0x81,
    INS_READ_DATA_BLOCK = 0x82,
    INS_CLEAR_DATA_BUFFER = 0x83,

    INS_SHOW_FLOW = 0x90,

    INS_PREPARE_BLINDSIGNING = 0x91,

    INS_PREPARE_SIGNING = 0xa0,
    INS_GENERATE_ADDRESS = 0xa1,
    INS_SIGN = 0xa2,
    INS_USER_CONFIRM_ESSENCE = 0xa3,

    INS_SIGN_SINGLE = 0xa4,

#ifdef APP_DEBUG
    INS_DUMP_MEMORY = 0x66,
    INS_SET_NON_INTERACTIVE_MODE = 0x67,
#endif

    INS_RESET = 0xff,
};

enum {
    SW_OK = 0x9000,
    SW_INCORRECT_LENGTH = 0x6700,
    SW_COMMAND_INVALID_DATA = 0x6a80,
    SW_INCORRECT_P1P2 = 0x6b00,
    SW_INCORRECT_LENGTH_P3 = 0x6c00,
    SW_INS_NOT_SUPPORTED = 0x6d00,
    SW_CLA_NOT_SUPPORTED = 0x6e00,

    SW_ACCOUNT_NOT_VALID = 0x6388,

    SW_COMMAND_NOT_ALLOWED = 0x6900,
    SW_FEATURE_NOT_SUPPORTED = 0x6901,
    SW_SECURITY_STATUS_NOT_SATISFIED = 0x6982,
    SW_CONDITIONS_OF_USE_NOT_SATISFIED = 0x6985,

    SW_COMMAND_TIMEOUT = 0x6401,
    SW_UNKNOWN = 0x69a0,

    SW_DENIED_BY_USER = SW_CONDITIONS_OF_USE_NOT_SATISFIED,
    SW_DEVICE_IS_LOCKED = SW_SECURITY_STATUS_NOT_SATISFIED,
};

#endif // IOTA_IO_H
