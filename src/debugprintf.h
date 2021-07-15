/*
 * debugprintf.h
 *
 *  Created on: 15.10.2020
 *      Author: thomas
 */

#ifndef SRC_DEBUGPRINTF_H_
#define SRC_DEBUGPRINTF_H_

//#define APP_DEBUG

#ifdef APP_DEBUG

#include <stdint.h>

#define DEBUG_BUFFER_SIZE 50
extern char G_debug_print_buf[DEBUG_BUFFER_SIZE];

void debug_write(char *buf);

void debug_print_sp(void);

// use define to avoid vsnprintf (would need _sbrk what is dynamic memory
// allocation)
#define debug_printf(fmt, ...)                                                 \
    {                                                                          \
        snprintf(G_debug_print_buf, sizeof(G_debug_print_buf) - 3, fmt,        \
                 ##__VA_ARGS__);                                               \
        debug_write(G_debug_print_buf);                                        \
    }

void debug_print_hex(const uint8_t *data, int size, int b);

#endif
#endif /* SRC_DEBUGPRINTF_H_ */
