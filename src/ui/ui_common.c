#include "ui/ui_common.h"
#include <stdint.h>
#include <string.h>
#include "api.h"
#include "macros.h"
#include "os.h"

#define BIP32_LINE_LENGTH 22

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

#define MUST(c)                                                                \
    {                                                                          \
        if (!(c)) {                                                            \
            return 0;                                                          \
        }                                                                      \
    }


/// the largest power of 10 that still fits into int32
#define MAX_INT_DEC UINT64_C(1000000000)

// the max number of iota units
#define MAX_IOTA_UNIT 6

/// the different IOTA units
static const char IOTA_UNITS[MAX_IOTA_UNIT][3] = {"i",  "Ki", "Mi",
                                                  "Gi", "Ti", "Pi"};

/// Groups the string by adding a comma every 3 chars from the right.
static size_t str_add_commas(char *dst, const char *src, size_t num_len)
{
    char *p_dst = dst;
    const char *p_src = src;

    // ignore leading minus
    if (*p_src == '-') {
        *p_dst++ = *p_src++;
        num_len--;
    }
    for (int commas = 2 - num_len % 3; *p_src; commas = (commas + 1) % 3) {
        *p_dst++ = *p_src++;
        if (commas == 1) {
            *p_dst++ = ',';
        }
    }
    // remove the last comma and zero-terminate
    *--p_dst = '\0';

    return (p_dst - dst);
}

/** @brief Writes signed integer to string.
 *  @return the number of chars that have been written
 */
static size_t format_s64(char *s, const size_t n, const uint64_t val)
{
    // we cannot display the full range of int64 with this function
    if (val >= MAX_INT_DEC * MAX_INT_DEC) {
        THROW(INVALID_PARAMETER);
    }

    if (val < MAX_INT_DEC) {
        snprintf(s, n, "%u", (uint32_t)val);
    }
    else {
        // emulate printing of integers larger than 32 bit
        snprintf(s, n, "%u%09u", (uint32_t)(val / MAX_INT_DEC),
                 (int)(val % MAX_INT_DEC));
    }
    return strnlen(s, n);
}

void format_value_full(char *s, const unsigned int n, const uint64_t val)
{
    char buffer[n];

    const size_t num_len = format_s64(buffer, sizeof(buffer), val);
    const size_t num_len_comma = num_len + (num_len - 1) / 3;

    // if the length with commas plus the unit does not fit
    // +3 = max unit length (i, Mi, Gi, Ti)
    if (num_len_comma + 3 > n) {
        snprintf(s, n, "%s %s", buffer, IOTA_UNITS[0]);
    }
    else {
        const size_t chars_written = str_add_commas(s, buffer, num_len);
        snprintf(s + chars_written, n - chars_written, " %s", IOTA_UNITS[0]);
    }
}

void format_value_full_decimals(char *s, const unsigned int n,
                                const uint64_t val)
{
    char buffer[n];

    const size_t num_len = format_s64(buffer, sizeof(buffer), val);

    // not enough space
    if (n < num_len + 2) {
        THROW(SW_UNKNOWN);
    }

    // format 0,xxxxxx
    if (val < 1000000ull) {
        snprintf(s, n, "0.%06u", (uint32_t)val);
        return;
    }

    // format yyyy,xxxxxx
    // insert comma at the right spot during copying
    char *src = buffer;
    char *dst = s;
    for (size_t i = 0; i < num_len; i++) {
        if (i == num_len - 6) {
            *dst++ = '.';
        }
        *dst++ = *src++;
    }
    *dst = 0;
}


void format_value_short(char *s, const unsigned int n, uint64_t val)
{
    if (val < 1000) {
        snprintf(s, n, "%u %s", (uint32_t)(val), IOTA_UNITS[0]);
        return;
    }

    unsigned int base = 1;
    while (val >= 1000 * 1000) {
        val /= 1000;
        base++;
    }
    if (base >= MAX_IOTA_UNIT) {
        THROW(INVALID_PARAMETER);
    }

    snprintf(s, n, "%u.%03u %s", (uint32_t)(val / 1000), (uint32_t)(val % 1000),
             IOTA_UNITS[base]);
}

// returns the length of hex string
static int hex_len(uint32_t v)
{
    int len = 8;
    while (v) {
        if ((v & 0xf0000000)) {
            return len;
        }
        v <<= 4;
        len -= 1;
    }
    return 1;
}

// format bip path to string
// doesn't use a local buffer but generates the string
// fitting in LINE_WIDTH characters directly
static int format_bip32(const uint32_t *b32, int linenr, char *out,
                        uint32_t out_max_len)
{
    int len[BIP32_PATH_LEN] = {0};
    for (int i = 0; i < BIP32_PATH_LEN; i++) {
        len[i] = hex_len(b32[i] & 0x7fffffff);
        if (i != BIP32_PATH_LEN - 1) {
            len[i] += 2; // gets added: '/
        }
        else {
            len[i] += 1; // gets added: '
        }
    }

    int ofs = 0;
    int curlen = 0;
    int lines = 0;

    if (out) {
        out[0] = 0;
    }
    for (int i = 0; i < BIP32_PATH_LEN; i++) {
        if (curlen + len[i] > BIP32_LINE_LENGTH) {
            curlen = 0;
            lines++;
        }
        if (lines == linenr) {
            if (out) {
                snprintf(&out[ofs], out_max_len,
                         (i != BIP32_PATH_LEN - 1) ? "%x'/" : "%x'",
                         b32[i] & 0x7fffffff);
            }
            ofs += len[i];
            out_max_len -= len[i];
        }
        curlen += len[i];
    }
    return ofs;
}

int format_bip32_with_line_breaks(const uint32_t *b32, char *out,
                                  int out_max_len)
{
    int ofs = 0;
    int written = 0;
    int last_zero = 0;

    // maximum of 3 lines
    for (int i = 0; i < 3; i++) {
        written = format_bip32(b32, i, &out[ofs], out_max_len);

        if (!written) {
            break;
        }
        if (last_zero) {
            // if previously something was written, we have to replace \0 with
            // \n
            out[last_zero] = '\n';
        }
        ofs += written;
        out_max_len -= written;

        last_zero = ofs;

        // skip \0
        ofs++;
        out_max_len--;
    }
    out[ofs] = 0;
    return ofs;
}

int string_insert_chars_each(const char *src, uint32_t src_size, char *dst,
                             uint32_t dst_size, int insert_after, int count,
                             char c)
{
    uint32_t src_len = strnlen(src, src_size);

    // enough space?
    if (dst_size < src_len + (src_len / insert_after) + 1) {
        return 0;
    }

    int ctr = 0;
    for (uint32_t i = 0; i < src_len; i++) {
        *dst++ = *src++;
        ctr++;
        if (count > 0 && ctr == insert_after) {
            ctr = 0;
            count--;
            *dst++ = c;
        }
    }
    *dst = 0;
    return 1;
}
