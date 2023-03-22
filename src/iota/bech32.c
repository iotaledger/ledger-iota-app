/* Copyright (c) 2017 Pieter Wuille
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "bech32.h"

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"


static uint32_t bech32_polymod_step(uint32_t pre)
{
    uint8_t b = pre >> 25;
    return ((pre & 0x1FFFFFF) << 5) ^ (-((b >> 0) & 1) & 0x3b6a57b2UL) ^
           (-((b >> 1) & 1) & 0x26508e6dUL) ^ (-((b >> 2) & 1) & 0x1ea119faUL) ^
           (-((b >> 3) & 1) & 0x3d4233ddUL) ^ (-((b >> 4) & 1) & 0x2a1462b3UL);
}

#define charset "qpzry9x8gf2tvdw0s3jn54khce6mua7l"

int bech32_encode(char *const output, size_t *const out_len,
                  const char *const hrp, const size_t hrp_len,
                  const uint8_t *const data, const size_t data_len)
{
    uint32_t chk = 1;
    size_t out_off = 0;
    const size_t out_len_max = *out_len;
    // hrp '1' data checksum
    const size_t final_out_len = hrp_len + data_len + 7;
    if (final_out_len > 108)
        return 0;
    // Note we want <=, to account for the null at the end of the string
    // i.e. equivalent to out_len_max < final_out_len + 1
    if (output == NULL || out_len_max <= final_out_len)
        return 0;
    if (hrp == NULL || hrp_len <= 0)
        return 0;
    if (data == NULL || data_len <= 0)
        return 0;
    for (size_t i = 0; i < hrp_len; ++i) {
        char ch = hrp[i];
        if (!(33 <= ch && ch <= 126))
            return 0;
        chk = bech32_polymod_step(chk) ^ (ch >> 5);
    }
    chk = bech32_polymod_step(chk);
    for (size_t i = 0; i < hrp_len; ++i) {
        char ch = hrp[i];
        chk = bech32_polymod_step(chk) ^ (ch & 0x1f);
        output[out_off++] = ch;
    }
    output[out_off++] = '1';
    for (size_t i = 0; i < data_len; ++i) {
        if (data[i] >> 5)
            return 0;
        chk = bech32_polymod_step(chk) ^ data[i];
        output[out_off++] = charset[data[i]];
    }
    for (size_t i = 0; i < 6; ++i) {
        chk = bech32_polymod_step(chk);
    }
    chk ^= 1;
    for (size_t i = 0; i < 6; ++i) {
        output[out_off++] = charset[(chk >> ((5 - i) * 5)) & 0x1f];
    }
    output[out_off] = 0;
    *out_len = out_off;
    return (out_off == final_out_len);
}

int base32_encode(uint8_t *const out, size_t *out_len, const uint8_t *const in,
                  const size_t in_len)
{
    const int outbits = 5;
    const int inbits = 8;
    const int pad = 1;
    uint32_t val = 0;
    int bits = 0;
    size_t out_idx = 0;
    const size_t out_len_max = *out_len;
    const uint32_t maxv = (((uint32_t)1) << outbits) - 1;
    for (size_t inx_idx = 0; inx_idx < in_len; ++inx_idx) {
        val = (val << inbits) | in[inx_idx];
        bits += inbits;
        while (bits >= outbits) {
            bits -= outbits;
            if (out_idx >= out_len_max)
                return 0;
            out[out_idx++] = (val >> bits) & maxv;
        }
    }
    if (pad) {
        if (bits) {
            if (out_idx >= out_len_max)
                return 0;
            out[out_idx++] = (val << (outbits - bits)) & maxv;
        }
    }
    else if (((val << (outbits - bits)) & maxv) || bits >= inbits) {
        return 0;
    }
    // Set out index
    *out_len = out_idx;
    return 1;
}
