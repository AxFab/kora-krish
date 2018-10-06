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
#include "keys.h"

#define _K2(n) n, n
#define _K4(n) n, n, n, n

int kdb_layout_US_international[128][4] = {
  { _K4(0) }, // 00
  { _K4(0) }, // 01
  { _K4(0) }, // 02
  { _K4(0) }, // 03
  { _K4(0) }, // 04
  { _K4(0) }, // 05
  { _K4(0) }, // 06
  { _K4(0) }, // 07
  { _K4(0) }, // 08
  { _K4(KEY_ESCAPE) }, // 09
  { '1', '!', '¡', '¹' }, // 0A
  { '2', '@', '²', KEY_NONE }, // 0B
  { '3', '#', '³', KEY_NONE }, // 0C
  { '4', '$', '¤', '£' }, // 0D
  { '5', '%', '€', KEY_NONE }, // 0E
  { '6', '^', '¼', KEY_NONE }, // 0F
  { '7', '&', '½', KEY_NONE }, // 10
  { '8', '*', '¾', KEY_NONE }, // 11
  { '9', '(', '‘', KEY_NONE }, // 12
  { '0', ')', '’', KEY_NONE }, // 13
  { '-', '_', '¥', KEY_NONE }, // 14
  { '=', '+', '×', '÷' }, // 15
  { _K4(KEY_BACKSPACE) }, // 16
  { _K4(KEY_TABS) }, // 17
  { 'q', 'Q', 'ä', 'Ä' }, // 18 - Q
  { 'w', 'W', 'å', 'Å' }, // 19
  { 'e', 'E', 'é', 'É' }, // 1A
  { 'r', 'R', '®', KEY_NONE }, // 1B
  { 't', 'T', 'þ', 'Þ' }, // 1C
  { 'y', 'Y', 'ü', 'Ü' }, // 1D
  { 'u', 'U', 'ú', 'Ú' }, // 1E
  { 'i', 'I', 'í', 'Í' }, // 1F
  { 'o', 'O', 'ó', 'Ó' }, // 20
  { 'p', 'P', 'ö', KEY_NONE }, // 21 - P
  { '[', '{', '«', KEY_NONE }, // 22
  { ']', '}', '»', KEY_NONE }, // 23
  { _K4(KEY_ENTER) }, // 24
  { _K4(KEY_CTRL_LF) }, // 25
  { 'a', 'A', 'á', 'Á' }, // 26 - A
  { 's', 'S', 'ß', '§' }, // 27
  { 'd', 'D', 'ð', 'Ð' }, // 28
  { 'f', 'F', _K2(KEY_NONE) }, // 29
  { 'g', 'G', _K2(KEY_NONE) }, // 2A
  { 'h', 'H', _K2(KEY_NONE) }, // 2B
  { 'j', 'J', _K2(KEY_NONE) }, // 2C
  { 'k', 'K', _K2(KEY_NONE) }, // 2D
  { 'l', 'L', 'ø', 'Ø' }, // 2E - L
  { ';', ':', '¶', '°' }, // 2F
  { -('\''), -('"'), '´', '¨' }, // 30
  { -('`'), -('~'), _K2(KEY_NONE) }, // 31
  { _K4(KEY_SHIFT_LF) }, // 32
  { _K4(0) }, // 33
  { 'z', 'Z', 'æ', 'Æ' }, // 34 - Z
  { 'x', 'X', _K2(KEY_NONE) }, // 35
  { 'c', 'C', '©', '¢' }, // 36
  { 'v', 'V', _K2(0) }, // 37
  { 'b', 'B', _K2(0) }, // 38
  { 'n', 'N', 'ñ', 'Ñ' }, // 39
  { 'm', 'M', 'µ', KEY_NONE }, // 3A - M
  { ',', '<', 'ç', 'Ç' }, // 3B
  { '.', '>', _K2(KEY_NONE) }, // 3C
  { '/', '?', '¿', KEY_NONE }, // 3D
  { _K4(KEY_SHIFT_RG) }, // 3E
  { _K2('*'), _K2(KEY_NONE) }, // 3F -- Num pad
  { _K4(KEY_ALT_LF) }, // 40
  { _K2(' '), _K2(KEY_NONE) }, // 41
  { _K4(KEY_CAPSLOCK) }, // 42
  { _K4(KEY_F1) }, // 43
  { _K4(KEY_F2) }, // 44
  { _K4(KEY_F3) }, // 45
  { _K4(KEY_F4) }, // 46
  { _K4(KEY_F5) }, // 47
  { _K4(KEY_F6) }, // 48
  { _K4(KEY_F7) }, // 49
  { _K4(KEY_F8) }, // 4A
  { _K4(KEY_F9) }, // 4B
  { _K4(KEY_F10) }, // 4C
  { _K4(KEY_NUMLOCK) }, // 4D
  { _K4(0) }, // 4E -- FN+PgDn on my keyboard!
  { '7', KEY_HOME, _K2(KEY_NONE) }, // 4F
  { '8', KEY_UP, _K2(KEY_NONE) }, // 50
  { '9', KEY_PAGE_UP, _K2(KEY_NONE) }, // 51
  { _K2('-'), _K2(KEY_NONE) }, // 52 -- Num pad
  { '4', KEY_LEFT, _K2(KEY_NONE) }, // 53
  { '7', KEY_NONE, _K2(KEY_NONE) }, // 54
  { '6', KEY_RIGHT, _K2(KEY_NONE) }, // 55
  { _K2('+'), _K2(KEY_NONE) }, // 56 -- Num pad
  { '1', KEY_END, _K2(KEY_NONE) }, // 57
  { '2', KEY_DOWN, _K2(KEY_NONE) }, // 58
  { '3', KEY_PAGE_DOWN, _K2(KEY_NONE) }, // 59
  { '0', KEY_INSERT, _K2(KEY_NONE) }, // 5A
  { '.', KEY_DELETE, _K2(KEY_NONE) }, // 5B
  { _K4(0) }, // 5C
  { _K4(0) }, // 5D
  { '\\', '|', _K2(KEY_NONE) }, // 5E
  { _K4(KEY_F11) }, // 5F
  { _K4(KEY_F12) }, // 60
  { _K4(0) }, // 61
  { _K4(0) }, // 62
  { _K4(0) }, // 63
  { _K4(0) }, // 64
  { _K4(0) }, // 65
  { _K4(0) }, // 66
  { _K4(0) }, // 67
  { _K4(KEY_ENTER) }, // 68 -- Num pad
  { _K4(0) }, // 69
  { _K2('/'), _K2(KEY_NONE) }, // 6A -- Num pad
  { _K4(0) }, // 6B
  { _K4(KEY_ALT_RG) }, // 6C
  { _K4(0) }, // 6D
  { _K4(KEY_HOME) }, // 6E
  { _K4(KEY_UP) }, // 6F
  { _K4(KEY_PAGE_UP) }, // 70
  { _K4(KEY_LEFT) }, // 71
  { _K4(KEY_RIGHT) }, // 72
  { _K4(KEY_END) }, // 73
  { _K4(KEY_DOWN) }, // 74
  { _K4(KEY_PAGE_DOWN) }, // 75
  { _K4(KEY_INSERT) }, // 76
  { _K4(KEY_DELETE) }, // 77
  { _K4(0) }, // 78
  { _K4(0) }, // 79
  { _K4(0) }, // 7A
  { _K4(0) }, // 7B
  { _K4(0) }, // 7C
  { _K4(0) }, // 7D
  { _K4(0) }, // 7E
  { _K4(KEY_BREAK) }, // 7F
  // { _K4(0) }, // 80
  // { _K4(0) }, // 81
  // { _K4(0) }, // 82
  // { _K4(0) }, // 83
  // { _K4(0) }, // 84
  // { _K4(KEY_HOST) }, // 85
  // { _K4(0) }, // 86
  // { _K4(KEY_MENU) }, // 87
};

int dead_keys_US_international(int first, int second)
{
  if (second == ' ') {
    return first;
  } else if (first == second) {
    return second; // We should write it two times !?
  }

  if (first == '\'') {
    switch (second) {
      case 'a': return 'á';
      case 'e': return 'é';
      case 'i': return 'í';
      case 'o': return 'ó';
      case 'u': return 'ú';
      case 'y': return 'ý';
      case 'c': return 'ç';
      case 'A': return 'Á';
      case 'E': return 'É';
      case 'I': return 'Í';
      case 'O': return 'Ó';
      case 'U': return 'Ú';
      case 'Y': return 'Ý';
      case 'C': return 'Ç';
    }
  } else if (first == '"') {
    switch (second) {
      case 'a': return 'ä';
      case 'e': return 'ë';
      case 'i': return 'ï';
      case 'o': return 'ö';
      case 'u': return 'ü';
      case 'y': return 'ÿ';
      case 'A': return 'Ä';
      case 'E': return 'Ë';
      case 'I': return 'Ï';
      case 'O': return 'Ö';
      case 'U': return 'Ü';
    }
  } else if (first == '`') {
    switch (second) {
      case 'a': return 'à';
      case 'e': return 'è';
      case 'i': return 'ì';
      case 'o': return 'ò';
      case 'u': return 'ù';
      case 'A': return 'À';
      case 'E': return 'È';
      case 'I': return 'Ì';
      case 'O': return 'Ò';
      case 'U': return 'Ù';
    }
  } else if (first == '~') {
    switch (second) {
      case 'a': return 'ã';
      case 'e': return 'õ';
      case 'n': return 'ñ';
      case 'A': return 'Ã';
      case 'E': return 'Õ';
      case 'N': return 'Ñ';
    }
  }

  // printf("Try to combine '%x' with '%x'\n", first, second);
  return first; // TODO write both
}

