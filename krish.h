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
#ifndef __KRISH_H
#define __KRISH_H 1

#include <stdlib.h>
#include <gfx.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

typedef int uchar_t;

#define UC_LEN_MAX 6
int uclen(const char *str, size_t lg);
int mbtouc(uchar_t *unicode, const char *str, size_t lg);
int uctomb(char *str, uchar_t unicode);


typedef struct termio termio_t;
typedef struct job job_t;


struct shell_cmd {
    char *name_;
    int argc_;
    char *argv_[64];
    int in_;
    int out_;
    int err_;
    int flags_;
    char *in_name_;
    char *out_name_;
    char *err_name_;
    struct shell_cmd *prev_;
};


#define SH_BACKGROUND (1 << 0)

#define SH_REDIRECT_IN (1 << 1)
#define SH_REDIRECT_OUT (1 << 2)
#define SH_REDIRECT_ERR (1 << 3)

#define SH_TRUNCAT_IN (1 << 4)
#define SH_TRUNCAT_OUT (1 << 5)
#define SH_TRUNCAT_ERR (1 << 6)

#define SH_ERROR (1 << 7)


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

termio_t *terminal_create(void(*readline)(termio_t *, const char *));
void terminal_destroy(termio_t *tty);

void terminal_write(termio_t *tty, const unsigned char *buf, size_t len);
void terminal_scroll(termio_t *tty, int speed);
bool terminal_redraw(termio_t *tty);
void terminal_paint(termio_t *tty);
void terminal_key(termio_t *tty, uchar_t unicode, int status);
void terminal_prompt(termio_t *tty, const char *buf);
void terminal_puts(termio_t *tty, const char *buf);
void terminal_select(termio_t *tty, int start_row, int start_col, int end_row, int end_col);

void terminal_push_job(termio_t *tty, job_t *job);
void terminal_pop_job(termio_t *tty, job_t *job);
job_t *terminal_job(termio_t *tty);
void terminal_resize(termio_t *tty, gfx_t *win);
void terminal_inval_all(termio_t *tty);
int terminal_copy(termio_t *tty, char *buf, int lg);
int terminal_paste(termio_t *tty, const char *buf, int len);
void terminal_font_size(termio_t *tty, int *w, int *h);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void job_init();
void job_dump();
job_t *job_by_pid(int pid);
job_t *job_create(termio_t *tty, const char *name);
void job_destroy(job_t *job);
void job_args(job_t *job, int argc, char **argv);
void job_start(job_t *job);
void job_close_stdin(job_t *job);
void job_puts(job_t *job, const char *str);
job_t *job_background(job_t *job);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

struct shell_cmd *Shell_cmd();
int Shell_fifo();
void Shell_stash_push();
void Shell_stash_pop();
void Shell_sweep();
int Shell_run();
void Shell_error(char *error, char *token);
void Shell_stack_arg(char *token);
void Shell_free_cmd(struct shell_cmd *cmd);

void parse_token(char *token);
void parse_flush();
char *parse_tokenize(const char *line, const char **sreg);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void shell_prompt(termio_t *tty);
int __exec(char *name, const char **argv, const char **env, int fds[3]);

#endif  /* __KRISH_H */
