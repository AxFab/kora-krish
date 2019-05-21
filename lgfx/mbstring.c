#include <stddef.h>
#include "../krish.h"

int uclen(const char *str, size_t lg)
{
    int i, len = 1;
    unsigned s = (unsigned char)(*str);
    if (s >= 0xfe)
        return -1;
    else if (s >= 0xfc)
        len = 6;
    else if (s >= 0xf8)
        len = 5;
    else if (s >= 0xf0)
        len = 4;
    else if (s >= 0xe0)
        len = 3;
    else if (s >= 0xc0)
        len = 2;
    else if (s >= 0x80)
        return -1;
    if (lg < (unsigned)len)
        return -1;

    for (i = 1; i < len; ++i) {
        if ((str[i] & 0xc0) != 0x80)
            return -1;
    }
    return len;
}


int mbtouc(uchar_t *unicode, const char *str, size_t lg)
{
    int len = uclen(str, lg);
    if (len == 1) {
        *unicode = *str;
        return 1;
    }
    if (len < 1)
        return -1;
    int l = 0, i = len;
    int wc = 0;
    while (i-- > 1)
        wc |= (((unsigned char)str[i] & 0x3f) << (6 * l++));
    wc |= (((unsigned char)str[0]) & ((1 << (6 - l)) - 1)) << (6 * l);
    *unicode = wc;
    return len;
}


int uctomb(char *str, uchar_t unicode)
{
    if (unicode < 0)
        return -1;
    else if (unicode < 0x80) {
        str[0] = unicode & 0x7f;
        return 1;
    } else if (unicode < 0x800) {
        str[0] = 0xc0 | ((unicode >> 6) & 0x1f);
        str[1] = 0x80 | (unicode & 0x3f);
        return 2;
    } else if (unicode < 0x10000) {
        str[0] = 0xe0 | ((unicode >> 12) & 0x0f);
        str[1] = 0x80 | ((unicode >> 6) & 0x3f);
        str[2] = 0x80 | (unicode & 0x3f);
        return 3;
    } else if (unicode < 0x400000) {
        str[0] = 0xf0 | ((unicode >> 18) & 0x07);
        str[1] = 0x80 | ((unicode >> 12) & 0x3f);
        str[2] = 0x80 | ((unicode >> 6) & 0x3f);
        str[3] = 0x80 | (unicode & 0x3f);
        return 4;
    } else if (unicode < 0x4000000) {
        str[0] = 0xf8 | ((unicode >> 24) & 0x03);
        str[1] = 0x80 | ((unicode >> 18) & 0x3f);
        str[2] = 0x80 | ((unicode >> 12) & 0x3f);
        str[3] = 0x80 | ((unicode >> 6) & 0x3f);
        str[4] = 0x80 | (unicode & 0x3f);
        return 5;
    } else {
        str[0] = 0xfc | ((unicode >> 30) & 0x01);
        str[1] = 0x80 | ((unicode >> 24) & 0x3f);
        str[2] = 0x80 | ((unicode >> 18) & 0x3f);
        str[3] = 0x80 | ((unicode >> 12) & 0x3f);
        str[4] = 0x80 | ((unicode >> 6) & 0x3f);
        str[5] = 0x80 | (unicode & 0x3f);
        return 6;
    }
}

