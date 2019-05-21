/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2018  <Fabien Bavent>
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
#include "gfx.h"
#include <stdio.h>
#include <string.h>

#define K4(n)  { n, n, 0, 0 }
#define K2(a,b)  { a, b, 0, 0 }

#define k2(n)  { n, n }

#define _K2(n) n, n
#define _K4(n) n, n, n, n

int kdb_layout_US_international[128][4] = {
    K4(0), // 00
    K4('\033'), // 01 - Escape
    { '1', '!', 0xA1, 0xB9 }, // 02
    { '2', '@', 0xB2, 0 }, // 03
    { '3', '#', 0xB3, 0 }, // 04
    { '4', '$', 0xA4, 0xA3 }, // 05
    { '5', '%', 0x20AC, 0 }, // 06
    { '6', '^', 0xBC, 0 }, // 07
    { '7', '&', 0xBD, 0 }, // 08
    { '8', '*', 0xBE, 0 }, // 09
    { '9', '(', 0x2018, 0 }, // 0A
    { '0', ')', 0x2019, 0 }, // 0B
    { '-', '_', 0xA5, 0 }, // 0C
    { '=', '+', 0xD7, 0xF7 }, // 0D
    K4(8), // 0E - Backspace
    K4('\t'), // 0F - Tab
    { 'q', 'Q', 0xE4, 0xC4 }, // 10 - Q
    { 'w', 'W', 0xE5, 0xC5 }, // 11
    { 'e', 'E', 0xC9, 0xC9 }, // 12
    { 'r', 'R', 0xAE, 0 }, // 13
    { 't', 'T', 0xFE, 0xDE }, // 14
    { 'y', 'Y', 0xFC, 0xDC }, // 15
    { 'u', 'U', 0xFA, 0xDA }, // 16
    { 'i', 'I', 0xED, 0xCD }, // 17
    { 'o', 'O', 0xF3, 0xD3 }, // 18
    { 'p', 'P', 0xF6, 0 }, // 19 - P
    { '[', '{', 0xAB, 0 }, // 1A
    { ']', '}', 0xBB, 0 }, // 1B
    { _K4('\n') }, // 1C
    { _K4(KEY_CTRL_L) }, // 1D
    { 'a', 'A', 0xE1, 0xC1 }, // 1E - A
    { 's', 'S', 0xDF, 0xA7 }, // 1F
    { 'd', 'D', 0xF0, 0xD0 }, // 20
    { 'f', 'F', _K2(0) }, // 21
    { 'g', 'G', _K2(0) }, // 22
    { 'h', 'H', _K2(0) }, // 23
    { 'j', 'J', _K2(0) }, // 24
    { 'k', 'K', _K2(0) }, // 25
    { 'l', 'L', 0xF8, 0xD8 }, // 26 - L
    { ';', ':', 0xB6, 0xB0 }, // 27
    { -('\''), -('"'), 0xB4, 0xA8 }, // 28
    { -('`'), -('~'), _K2(0) }, // 29
    { _K4(KEY_SHIFT_L) }, // 2A
    K4(0), // 2B
    { 'z', 'Z', 0xE6, 0xC6 }, // 2C - Z
    { 'x', 'X', _K2(0) }, // 2D
    { 'c', 'C', 0xA9, 0xA2 }, // 2E
    { 'v', 'V', _K2(0) }, // 2F
    { 'b', 'B', _K2(0) }, // 30
    { 'n', 'N', 0xF1, 0xD1 }, // 31
    { 'm', 'M', 0xB5, 0 }, // 32 - M
    { ',', '<', 0xE7, 0xC7 }, // 33
    { '.', '>', _K2(0) }, // 34
    { '/', '?', 0xBF, 0 }, // 35
    { _K4(KEY_SHIFT_R) }, // 36
    K4('*'), // 37 -- Num pad
    { _K4(KEY_ALT_L) }, // 38
    K4(' '), // 39
    { _K4(KEY_CAPS_LOCK) }, // 3A
    { _K4(KEY_F1) }, // 3B
    { _K4(KEY_F2) }, // 3C
    { _K4(KEY_F3) }, // 3D
    { _K4(KEY_F4) }, // 3E
    { _K4(KEY_F5) }, // 3F
    { _K4(KEY_F6) }, // 40
    { _K4(KEY_F7) }, // 41
    { _K4(KEY_F8) }, // 42
    { _K4(KEY_F9) }, // 43
    { _K4(KEY_F10) }, // 44
    { _K4(KEY_NUM_LOCK) }, // 45
    K4(0), // 46 -- FN+PgDn on my keyboard!
    { '7', KEY_HOME, _K2(0) }, // 47
    { '8', KEY_UP, _K2(0) }, // 48
    { '9', KEY_PAGE_UP, _K2(0) }, // 49
    { _K2('-'), _K2(0) }, // 4A -- Num pad
    { '4', KEY_LEFT, _K2(0) }, // 4B
    { '7', 0, _K2(0) }, // 4C
    { '6', KEY_RIGHT, _K2(0) }, // 4D
    { _K2('+'), _K2(0) }, // 4E -- Num pad
    { '1', KEY_END, _K2(0) }, // 4F
    { '2', KEY_DOWN, _K2(0) }, // 50
    { '3', KEY_PAGE_DOWN, _K2(0) }, // 51
    { '0', KEY_INSERT, _K2(0) }, // 52
    { '.', KEY_DELETE, _K2(0) }, // 53
    K4(0), // 54
    K4(0), // 55
    { '\\', '|', _K2(0) }, // 56
    { _K4(KEY_F11) }, // 57
    { _K4(KEY_F12) }, // 58
    K4(0), // 59
    K4(0), // 5A
    K4(0), // 5B
    K4(0), // 5C
    K4(0), // 5D
    K4(0), // 5E
    K4(0), // 5F
    { _K4('\n') }, // 60 -- Num pad
    K4(0), // 61
    K4('/'), // 62 -- Num pad
    K4(0), // 63
    { _K4(KEY_ALT_R) }, // 64
    K4(0), // 65
    { _K4(KEY_HOME) }, // 66
    { _K4(KEY_UP) }, // 67
    { _K4(KEY_PAGE_UP) }, // 68
    { _K4(KEY_LEFT) }, // 69
    { _K4(KEY_RIGHT) }, // 6A
    { _K4(KEY_END) }, // 6B
    { _K4(KEY_DOWN) }, // 6C
    { _K4(KEY_PAGE_DOWN) }, // 6D
    { _K4(KEY_INSERT) }, // 6E
    { _K4(KEY_DELETE) }, // 6F
    K4(0), // 70
    K4(0), // 71
    K4(0), // 72
    K4(0), // 73
    K4(0), // 74
    K4(0), // 75
    K4(0), // 76
    { _K4(KEY_BREAK) }, // 77
    K4(0), // 78
    K4(0), // 79
    K4(0), // 7A
    K4(0), // 7B
    K4(0), // 7C
    K4(0), // 7D
    K4(0), // 7E
    K4(0), // 7F
};

int latin_alternate[8][6][2] = {
    // grave, acute, circumflex, diaersis, tilde, ring
    // e
    { { 0xE8, 0xC8 }, { 0xE9, 0xC9 }, { 0xEA, 0xCA }, { 0xEB, 0xCB }, { 0, 0 }, { 0, 0 } },
    // y
    { { 0, 0 }, { 0xFD, 0xDD }, { 0, 0 }, { 0xFF, 0 }, { 0, 0 }, { 0, 0 } },
    // u
    { { 0xF9, 0xD9 }, { 0xFA, 0xDA }, { 0xFB, 0xDB }, { 0xFC, 0xDC }, { 0, 0 }, { 0, 0 } },
    // i
    { { 0xEC, 0xCC }, { 0xED, 0xCD }, { 0xEE, 0xCE }, { 0xEF, 0xCF }, { 0, 0 }, { 0, 0 } },
    // o
    { { 0xF2, 0xD2 }, { 0xF3, 0xD3 }, { 0xF4, 0xD4 }, { 0xF6, 0xD6 }, { 0xF5, 0xD5 }, { 0, 0 } },
    // a
    { { 0xE0, 0xC0 }, { 0xE1, 0xC1 }, { 0xE2, 0xC2 }, { 0xE4, 0xC4 }, { 0xE3, 0xC3 }, { 0xE5, 0xC5 } },
    // c
    { { 0, 0 }, { 0xE7, 0xC7 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    // n
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0xF1, 0xD1 }, { 0, 0 } },
};

int peek_key = 0;

int keyboard_down(int key, int *status, int *key2)
{
    *key2 = 0;
    if (key >= 128) {
        printf("KEY %d\n", key);
        return 0;
    }

    int s = (*status) & KDB_SHIFT ? 1 : 0;
    if ((*status) & KDB_CAPSLOCK)
        s = 1 - s;

    if (*(status) & KDB_ALT) {
        peek_key = 0;
        s += 2;
    } else if (peek_key != 0) { // Alternative keys
        int ch = kdb_layout_US_international[key][0];
        if (key == '\033') { // Escape
            peek_key = 0;
            return 0;
        } else if (ch == ' ') { // Space
            s = peek_key;
            peek_key = 0;
            return s;
        }

        do {
            static const char *sp_letters = "eyuioacn";
            static const char *sp_puncs = "`'\"^~Â";
            char *di = strchr(sp_letters, ch);
            if (di == NULL)
                break;
            int i = di - sp_letters;
            char *dj = strchr(sp_puncs, peek_key);
            if (dj == NULL)
                break;
            int j = dj - sp_puncs;
            if (i >= 0 && j >= 0 && latin_alternate[i][j][s] > 0) {
                peek_key = 0;
                return latin_alternate[i][j][s];
            }
        } while(0);

        *key2 = peek_key;
        peek_key = 0;
        return ch;
    }

    printf("Key %02x.%d => %04x\n", key, s, kdb_layout_US_international[key][s]);
    key = kdb_layout_US_international[key][s];
    if (key < 0) {
        peek_key = -key;
        return 0;
    }

    if (key < 0x20) {
        if (key == KEY_SHIFT_L || key == KEY_SHIFT_R)
            *status |= KDB_SHIFT;
        else if (key == KEY_ALT_L || key == KEY_ALT_R)
            *status |= KDB_ALT;
        else if (key == KEY_CTRL_L || key == KEY_CTRL_R)
            *status |= KDB_CTRL;
        else if (key == KEY_CAPS_LOCK)
            *status ^= KDB_CAPSLOCK;
    }
    return key;
}

int keyboard_up(int key, int *status)
{
    if (key >= 128)
        return 0;
    int s = (*status) & KDB_SHIFT ? 1 : 0;
    if ((*status) & KDB_CAPSLOCK)
        s = 1 - s;
    key = kdb_layout_US_international[key][s];
    if (key <= 0)
        return 0;
    if (key < 0x20) {
        if (key == KEY_SHIFT_L || key == KEY_SHIFT_R)
            *status &= ~KDB_SHIFT;
        else if (key == KEY_ALT_L || key == KEY_ALT_R)
            *status &= ~KDB_ALT;
        else if (key == KEY_CTRL_L || key == KEY_CTRL_R)
            *status &= ~KDB_CTRL;
    }
    return key;
}

