#include <assert.h>
#include "gfx.h"

static void *memcpy32(void *dest, void *src, size_t lg)
{
    assert(IS_ALIGNED(lg, 4));
    assert(IS_ALIGNED((size_t)dest, 4));
    assert(IS_ALIGNED((size_t)src, 4));
    register uint32_t *a = (uint32_t *)src;
    register uint32_t *b = (uint32_t *)dest;
    while (lg > 16) {
        b[0] = a[0];
        b[1] = a[1];
        b[2] = a[2];
        b[3] = a[3];
        lg -= 16;
        a += 4;
        b += 4;
    }
    while (lg > 0) {
        b[0] = a[0];
        lg -= 4;
        a++;
        b++;
    }
    return dest;
}

static void *memset32(void *dest, uint32_t val, size_t lg)
{
    assert(IS_ALIGNED(lg, 4));
    assert(IS_ALIGNED((size_t)dest, 4));
    register uint32_t *a = (uint32_t *)dest;
    while (lg > 16) {
        a[0] = val;
        a[1] = val;
        a[2] = val;
        a[3] = val;
        lg -= 16;
        a += 4;
    }
    while (lg > 0) {
        a[0] = val;
        lg -= 4;
        a++;
    }
    return dest;
}


void gfx_rect(gfx_t *gfx, int x, int y, int w, int h, uint32_t color)
{
    int j;
    int minx = MAX(0, x);
    int maxx = MIN(gfx->width, x + w);
    int miny = MAX(0, y);
    int maxy = MIN(gfx->height, y + h);

    for (j = miny; j < maxy; ++j)
        memset32(&gfx->pixels[j * gfx->pitch + minx * 4], color, (maxx - minx) * 4);
}


void gfx_clear(gfx_t *gfx, uint32_t color)
{
    memset32(gfx->pixels, color, gfx->pitch * gfx->height);
}


void gfx_glyph(gfx_t *gfx, const font_bmp_t *font, uint32_t unicode, uint32_t fg, uint32_t bg, int x, int y)
{
    int l, px, ph;
    int py = y;
    int gx = MIN(gfx->width, x + font->width);
    int sx = MIN(gfx->width, x + font->dispx);
    int gy = MIN(gfx->height, y + font->height);
    int sy = MIN(gfx->height, y + font->dispy);
    const uint8_t *glyph = &font->glyphs[(unicode - 0x20) * font->glyph_size];
    for (l = 0; py < gy; ++py) {
        ph = py * gfx->width;
        for (px = x; px < gx; ++px, ++l)
            gfx->pixels4[ph + px] = (glyph[l / 8] & (1 << l % 8)) ? fg : bg;
        for (; px < sx; ++px)
            gfx->pixels4[ph + px] = bg;
        // TODO -- l is late if gx < x + font->width
    }

    for (; py < sy; ++py) {
        ph = py * gfx->width;
        for (px = x; px < sx; ++px)
            gfx->pixels4[ph + px] = bg;
    }
}


void gfx_copy(gfx_t *dest, gfx_t *src, int x, int y, int w, int h)
{
    int i;
    int minx = MAX(0, -x);
    int maxx = MIN(MIN(w, src->width), dest->width - x);
    int miny = MAX(0, -y);
    int maxy = MIN(MIN(h, src->height), dest->height - y);

    for (i = miny; i < maxy; ++i) {
        int ka = i * src->pitch;
        int kb = (i + y) * dest->pitch;
        memcpy32(&dest->pixels[kb + (minx + x) * 4], &src->pixels[ka + minx * 4], (maxx - minx) * 4);
    }
}


