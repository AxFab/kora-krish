/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2021  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 */
#ifndef _FONTS_H
#define _FONTS_H 1

#include <gfx.h>

typedef struct font_bmp font_bmp_t;

struct font_bmp {
    const uint8_t *glyphs;
    char glyph_size;
    char width;
    char height;
    char dispx;
    char dispy;
};

// void gfx_glyph(gfx_t *gfx, const font_bmp_t *font, uint32_t unicode, uint32_t fg, uint32_t bg, int x, int y);

#endif  /* _FONTS_H */
