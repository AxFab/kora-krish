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
#include "shell.h"
#include "kdb/keys.h"

struct kdb_layout {
  char *name_;
  int **layout_;
  int (*dead_keys)(int, int);
};

#define KeyboardLayout(n)  extern int kdb_layout_## n[128][4]; extern int dead_keys_ ## n (int f, int s);
#include "kdb/layouts.h"

#undef KeyboardLayout
#define KeyboardLayout(n) { #n, NULL, dead_keys_ ## n },

struct kdb_layout layouts[] = {
#include "kdb/layouts.h"
};


int kdb_state = 0;
int kdb_dead = 0;
int kdb_layout = 0;

static int Keyboard_layout(int code, int state)
{
  return kdb_layout_US_international[code][state & 3];
}

static int Keyboard_combine(int first, int second)
{
  return layouts[kdb_layout].dead_keys(first, second);
}

static int Keyboard_getValue(int code)
{
  if (code == 0x7F) {
    return KEY_BREAK;
  } else if (code == 0x85) {
    return KEY_HOST;
  } else if (code == 0x87) {
    return KEY_MENU;
  } else if (code < 0x80) {
    int value = Keyboard_layout(code, kdb_state);
    if (value > 0 && kdb_dead == 0) {
      return value;
    } else if (value > 0) {
      value = Keyboard_combine(kdb_dead, value);
      kdb_dead = 0;
      return value;
    } else {
      kdb_dead = -value;
    }
  }
  return KEY_NONE;
}

int Keyboard_state() {
  return kdb_state;
}

int Keyboard_release(int code)
{
  int value = code < 0x80 ? Keyboard_layout(code, kdb_state) : KEY_NONE;
  if (value == KEY_ALT_RG) {
    kdb_state &= ~2;
  } else if (value == KEY_SHIFT_LF || value == KEY_SHIFT_RG) {
    kdb_state &= ~1;
  } else if (value == KEY_CTRL_LF) {
    kdb_state &= ~4;
  }
  // if (value < 0x20 || value >= 0x80) {
  //   printf("Key %02x (%02x) released\n", code, value);
  // } else {
  //   printf("Key %02x ('%c') released\n", code, value);
  // }
  return value;
}

int Keyboard_press(int code)
{
  int value = Keyboard_getValue(code);
  if (value == KEY_ALT_RG) {
    kdb_state |= 2;
  } else if (value == KEY_SHIFT_LF || value == KEY_SHIFT_RG) {
    kdb_state |= 1;
  } else if (value == KEY_CTRL_LF) {
    kdb_state |= 4;
  }
  // if (value < 0x20 || value >= 0x80) {
  //   printf("Key %02x (%02x) press %02x\n", code, value, kdb_dead);
  // } else {
  //   printf("Key %02x ('%c') press %02x\n", code, value, kdb_dead);
  // }
  return value;
}
