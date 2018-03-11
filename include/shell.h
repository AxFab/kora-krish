#ifndef _SRC_SHELL_H
#define _SRC_SHELL_H 1

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <errno.h>
#include "tchar.h"

#define SHELL_BUFFER_SIZE (64 * 1024)

struct shell_cmd {
  TCHAR* name_;
  int argc_;
  TCHAR* argv_[64];
  int in_;
  int out_;
  int err_;
  int flags_;
  TCHAR *in_name_;
  TCHAR *out_name_;
  TCHAR *err_name_;
  struct shell_cmd *prev_;
};

struct term_line {
  uint32_t txColor_;
  uint32_t bgColor_;
  int offset_;
  int flags_;
  struct term_line *prev_;
  struct term_line *next_;
};

typedef void (*painter_f) (struct term_line *, int, int, int);

#define SH_BACKGROUND (1 << 0)
#define SH_ERROR (1 << 1)

#define SH_TRUNCAT_OUT (1 << 3)
#define SH_TRUNCAT_ERR (1 << 4)

int Environ_refresh();

struct shell_cmd *Shell_cmd();
int Shell_fifo ();
void Shell_stash_push();
void Shell_stash_pop();
void Shell_sweep();
int Shell_run();
void Shell_error(char *error, TCHAR *token);
void Shell_stack_arg(TCHAR *token);
void Shell_free_cmd(struct shell_cmd* cmd);

void Parse_token(TCHAR *token);
void Parse_flush();
TCHAR *Parse_tokenize(TCHAR *line, TCHAR **sreg);

TCHAR *Rdline_read(TCHAR *promt);

void History_push(TCHAR* line);

int Keyboard_state();
int Keyboard_release(int code);
int Keyboard_press(int code);

int Display_init();
void Display_close();
TCHAR *Display_promt(TCHAR *promt);
void Display_redraw();

int Termio_push_line(struct term_line *line, int row);
int Termio_write (TCHAR *str, int lg);
void Termio_init(int rows, int cols, painter_f painter);
struct term_line *Termio_top();
void Termio_sweep();
void Termio_scroll(int count);

#endif  /* _SRC_SHELL_H */
