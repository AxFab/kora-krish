#ifndef _FONTS_H
#define _FONTS_H 1

#include <kora/gfx.h>

typedef struct font_bmp font_bmp_t;

struct font_bmp {
    const uint8_t *glyphs;
    char glyph_size;
    char width;
    char height;
    char dispx;
    char dispy;
};

void gfx_glyph(gfx_t* gfx, const font_bmp_t* font, uint32_t unicode, uint32_t fg, uint32_t bg, int x, int y);

#endif  /* _FONTS_H */
