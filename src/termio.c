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
#include "fifo.h"

/* If set, indicate that what we write on output is in fact user input. */
#define TTY_ON_INPUT  (1 << 0)
/* If set, indicate that we have content into input buffer. */
#define TTY_NEW_INPUT (1 << 1)
/* Ask to recho the line the next time the program read. */
#define TTY_READ_ECHO (1 << 2)

#define TTY_KEY_CTRL (1 << 4)
#define TTY_KEY_ALT (1 << 5)
#define TTY_KEY_SHIFT (1 << 6)

#define _ESC 0x1B
#define _EOL '\n'


struct term_info {
  int row_;
  int flags_;
  int max_row_;
  int max_col_;

  struct term_line *first_;
  struct term_line *top_;
  struct term_line *last_;

  uint32_t txColor_;
  uint32_t bgColor_;

  fifo_t *fifo_;
  void *address_;
  size_t size_;

  painter_f painter_;
};


uint32_t Colors_console[] = {
  0xff181818, 0xffa61010, 0xff10a610, 0xffa66010,
  0xff1010a6, 0xffa610a6, 0xff10a6a6, 0xffa6a6a6,
  0xffffffff
};

uint32_t Colors_foreground[] = {
  0xff323232, 0xffd01010, 0xff10d010, 0xffd0d010,
  0xff1060d0, 0xffd010d0, 0xff10d0d0, 0xffd0d0d0,
  0xffffffff
};

uint32_t Colors_background[] = {
  0xff181818, 0xffa61010, 0xff10a610, 0xffa66010,
  0xff1010a6, 0xffa610a6, 0xff10a6a6, 0xffa6a6a6,
  0xffffffff
};

struct term_info *Termio_instance;

static void Termio_ANSI_change_color(struct term_line *line, int rule) {

  if (rule == 0) {
    line->txColor_ = Termio_instance->txColor_;
    line->bgColor_ = Termio_instance->bgColor_;
  } else if (rule < 29) {
  } else if (rule < 39) {
    line->txColor_ = Colors_console[rule - 30];
  } else if (rule < 49) {
    line->bgColor_ = Colors_background[rule - 40];
  } else if (rule < 89) {
  } else if (rule < 99) {
    line->txColor_ = Colors_foreground[rule - 90];
  } else {
  }
}

static void Termio_ANSI_cmd(struct term_line *line, const char **str)
{
  int i;
  int values[8];

  if (**str != '[') {
    return;
  }

  i = 0;
  do {
    (*str)++;
    values[i++] = (int)strtoul(*str, (char **)str, 10);
  } while (i < 8 && **str == ';');

  switch (**str) {
    case 'm':
      while (i > 0) {
        Termio_ANSI_change_color(line, values[--i]);
      }
      break;
  }

  (*str)++;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static int Termio_readchar(struct term_line *line, const char **str)
{
  if (**str < 0) {
    fprintf(stderr, "Terminal is only ASCII\n");
    (*str)++;
    return 0x80;
  } else if (**str >= 0x20) {
    return *(*str)++;
  } else if (**str == _ESC || **str == _EOL) {
    return *(*str)++;
  }

  (*str)++;
  return 0x80;
}


void Termio_scroll(int count)
{
  if (count > 0) {
    while (count != 0) {
      if (Termio_instance->top_->next_ == NULL)
        break;

      Termio_instance->top_ = Termio_instance->top_->next_;
      count--;
      Termio_instance->row_--;
    }

  } else {
    while (count != 0) {
      if (Termio_instance->top_->prev_ == NULL)
        break;

      Termio_instance->top_ = Termio_instance->top_->prev_;
      count++;
      Termio_instance->row_++;
    }
  }

  Display_redraw();
}

int Termio_push_line(struct term_line *line, int row)
{
  int ch;
  int col = 0;
  const char *base = (char *)Termio_instance->address_;
  const char *str = &base[line->offset_]; // String is the offset of the string

  while (*str) {
    ch = Termio_readchar(line, &str);

    if (ch == _EOL) {
      return str - base;
    } else if (ch == _ESC) {
      Termio_ANSI_cmd(line, &str);
      continue;
    }

    Termio_instance->painter_(line, ch, row, col++);
    if (col >= Termio_instance->max_col_) {
      return str - base; // We return the place of the new line
    }
    if (line->offset_ + col >= (int)Termio_instance->fifo_->wpen_) {
      return 0;
    }
  }

  return 0;
}

char out_buf[64];

int Termio_write (TCHAR *str, int lg)
{
  struct term_line *new_line;
  int pen;

  if (lg < 0) {
    lg = strlen ((const char*)str);
  }
  for (;;) {
    lg -= fifo_in(Termio_instance->fifo_, str, lg, FP_NOBLOCK);
    if (lg <= 0)
      break;
    printf("ENSURE TO REMOVE LOST LINES\n");
    fifo_out(Termio_instance->fifo_, out_buf, MAX(64, lg), FP_NOBLOCK);
  }

  for (;;) {

    if (Termio_instance->row_ > Termio_instance->max_row_) {
      Termio_scroll(Termio_instance->row_ - Termio_instance->max_row_);
    }

    pen = Termio_push_line(Termio_instance->last_, Termio_instance->row_);
    // if ((Termio_instance->flags_ & (TTY_NEW_INPUT | TTY_ON_INPUT)) == TTY_NEW_INPUT)
    //   Termio_instance->flags_ |= TTY_READ_ECHO;

    if (pen == 0)
      break;

    new_line = (struct term_line*)calloc(sizeof(struct term_line), 1);
    new_line->offset_ = pen;
    new_line->txColor_ = Termio_instance->last_->txColor_;
    new_line->bgColor_ = Termio_instance->last_->bgColor_;
    new_line->flags_ = Termio_instance->last_->flags_;
    Termio_instance->last_->next_ = new_line;
    new_line->prev_ = Termio_instance->last_;
    Termio_instance->last_ = new_line;
    Termio_instance->row_++;
  }

  return lg;
}


void Termio_init(int rows, int cols, painter_f painter)
{
  Termio_instance = malloc (sizeof(struct term_info));
  Termio_instance->size_ = SHELL_BUFFER_SIZE;
  Termio_instance->address_ = malloc(Termio_instance->size_);
  memset(Termio_instance->address_, 0, Termio_instance->size_);
  Termio_instance->fifo_ = fifo_init(Termio_instance->address_, Termio_instance->size_);

  Termio_instance->txColor_ = 0xffa6a6a6; // 0xff5c5c5c;
  Termio_instance->bgColor_ = 0xff181818; // 0xff323232;

  Termio_instance->row_ = 1;
  Termio_instance->flags_ = 0;
  Termio_instance->first_ = (struct term_line*)calloc(sizeof(struct term_line), 1);
  Termio_instance->first_->txColor_ = 0xffa6a6a6;
  Termio_instance->first_->bgColor_ = 0xff181818; // 0xff323232;
  Termio_instance->last_ = Termio_instance->first_;
  Termio_instance->top_ = Termio_instance->first_;

  Termio_instance->max_row_ = rows;
  Termio_instance->max_col_ = cols;

  Termio_instance->painter_ = painter;
}

struct term_line *Termio_top()
{
  return Termio_instance->top_;
}

void Termio_sweep()
{
  struct term_line *line;
  while (Termio_instance->first_) {
    line = Termio_instance->first_->next_;
    free (Termio_instance->first_);
    Termio_instance->first_ = line;
  }

  free(Termio_instance->fifo_);
  free(Termio_instance->address_);
  free(Termio_instance);
  Termio_instance = NULL;
}