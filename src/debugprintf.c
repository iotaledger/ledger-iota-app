/*
 * debugprintf.c
 *
 *  Created on: 15.10.2020
 *      Author: thomas
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "os.h"

#include "debugprintf.h"


#ifdef APP_DEBUG


char G_debug_print_buf[DEBUG_BUFFER_SIZE];

#define max(a, b) (a > b) ? a : b
#define min(a, b) (a < b) ? a : b

void debug_write(char *buf)
{
    asm volatile("movs r0, #0x04\n"
                 "movs r1, %0\n"
                 "svc      0xab\n" ::"r"(buf)
                 : "r0", "r1");
}


static void _bin2hex(uint32_t v, char *hex, int n)
{
    if (n < 1) {
        return;
    }
    for (int i = 0; i < n; i++) {
        uint32_t c = v & 0xf;
        if (c >= 0 && c <= 9) {
            // digit
            hex[((n - 1) - i)] = '0' + c;
        }
        else {
            hex[((n - 1) - i)] = 'a' + c - 10;
            // hex
        }
        v >>= 4;
    }
}

static void bin2hex(uint8_t bin, char *hex)
{
    _bin2hex(bin, hex, 2);
}

static void *fnGetSP(void)
{
    volatile unsigned long var = 0;
    return (void *)((unsigned long)&var + 4);
}

void debug_print_sp()
{
    uint32_t sp = (uint32_t)fnGetSP();
    memset(G_debug_print_buf, 0, sizeof(G_debug_print_buf));
    _bin2hex(sp, G_debug_print_buf, 8);
    debug_write(G_debug_print_buf);
}

void debug_print_hex(const uint8_t *data, int size, int b)
{
    int chunks = size / b;
    if (size % b) {
        chunks++;
    }
    for (int i = 0; i < chunks; i++) {
        int idx = 0;
        memset(G_debug_print_buf, 0, sizeof(G_debug_print_buf));

        int chunk = min(size, b);
        for (int j = 0; j < chunk; j++) {
            if (idx >= (int)sizeof(G_debug_print_buf)) {
                return;
            }
            bin2hex((uint32_t)*data++, &G_debug_print_buf[idx]);
            idx += 2;
            if (j != chunk - 1) {
                G_debug_print_buf[idx++] = ' ';
            }
        }
        debug_write(G_debug_print_buf);
        size -= chunk;
    }
}
#endif
