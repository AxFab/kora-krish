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
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
// #include <mbstring.h>
#include "krish.h"
#include <gfx.h>
#include <threads.h>

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

termio_t *__tty;
char prompt[512];

void shell_prompt(termio_t *tty)
{
    int i;
    // TODO - Only if no foreground process
    char pwd[215];
    char host[215];
    char user[215];
    getcwd(pwd, 215);
    strcpy(host, "dev-vm");
    strcpy(user, "admin");
    for (i = 0; pwd[i]; ++i) {
        if (pwd[i] == '\\')
            pwd[i] = '/';
    }
    // getlogin_r(user, 215);
    // gethostname(host, 215);
    sprintf(prompt, "\033[31m%s\033[0m@\033[94m%s\033[0m:\033[33m%s\033[0m> ", user, host, pwd);
    terminal_prompt(tty, prompt);
}

void on_readline(termio_t *tty, const char *buf)
{
    // printf("EXEC: %s\n", buf);
    if (*buf == '\0') {
        if (terminal_job(tty) == NULL)
            shell_prompt(tty);
        return;
    }

    const char *sreg = NULL;
    char *token = NULL;
    for (;;) {
        token = parse_tokenize(buf, &sreg);
        if (token == NULL)
            break;
        parse_token(token);
        free(token);
    }
    parse_flush();
    if (terminal_job(tty) == NULL)
        shell_prompt(tty);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
extern const int __clipboard_size;
extern char __clipboard_buffer[8192];
int lx = 0, ly = 0, rx = 0, ry = 0;

bool on_repaint(gfx_t *gfx, termio_t *tty, gfx_seat_t *seat)
{
    return terminal_redraw(tty);
}

void on_expose(gfx_t *gfx, termio_t *tty, gfx_seat_t *seat)
{
    terminal_paint(tty);
}

void on_resize(gfx_t *gfx, termio_t *tty)
{
    terminal_resize(tty, gfx);
}
//
//void on_key_up(gfx_t *gfx, termio_t *tty, gfx_seat_t *seat, int key_code)
//{
//    int key = keyboard_up(key_code, &seat->kdb_status);
//    // printf("KEY UP: %x  (%o)\n", key, seat->kdb_status);
//}
//
//void on_key_down(gfx_t *gfx, termio_t *tty, gfx_seat_t *seat, int key_code)
//{
//    int key, key2;
//    key = keyboard_down(key_code, &seat->kdb_status, &key2);
//    if (key2 != 0)
//        terminal_key(tty, key2, seat->kdb_status);
//    // printf("KEY DW: %x  (%o)\n", key, seat->kdb_status);
//    terminal_key(tty, key, seat->kdb_status);
//}


void on_mse_up(gfx_t *gfx, termio_t *tty, int btn)
{
    int w, h;
    terminal_font_size(tty, &w, &h);
    if (btn == 1) {
        rx = gfx->seat->mouse_x / w;
        ry = gfx->seat->mouse_y / h;
        terminal_select(tty, ly, lx, ry, rx);
    }
}

void on_mse_down(gfx_t *gfx, termio_t *tty, int btn)
{
    if (btn == 1) {
        int w, h;
        terminal_font_size(tty, &w, &h);
        lx = rx = gfx->seat->mouse_x / w;
        ly = ry = gfx->seat->mouse_y / h;
    } else if (btn == 2) {

    } else if (btn == 3) {
        // TODO - Use non primary clipboard?
        int len = terminal_copy(tty, __clipboard_buffer, __clipboard_size);
        if (len > 0)
            terminal_paste(tty, __clipboard_buffer, len);
    }
}

void on_mse_move(gfx_t *gfx, termio_t *tty)
{
    if (gfx->seat->btn_status & 1) {
        int w, h;
        terminal_font_size(tty, &w, &h);
        rx = gfx->seat->mouse_x / w;
        ry = gfx->seat->mouse_y / h;
        terminal_select(tty, ly, lx, ry, rx);
    }
}

void on_mse_wheel(gfx_t *gfx, termio_t *tty, int disp)
{
    terminal_scroll(tty, disp);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */



int main(int argc, char const *argv[])
{
    job_init();
    termio_t *tty = terminal_create(on_readline);
    __tty = tty;
#ifdef _WIN32
    gfx_context("win32");
#endif
#ifndef __kora__
    gfx_t *win = gfx_create_window(_16x10(480), 480);
#else
    gfx_t *win = gfx_open_surface("/dev/fb0");
    gfx_open_input("/dev/kbd", win->uid);
#endif

    terminal_resize(tty, win);

    gfx_map(win);
    terminal_puts(tty, "  Welcome on \033[96mKrish\033[0m, \033[90;43mv0.1.3\033[0m\n");
    shell_prompt(tty);
    gfx_timer(20, 20);
    gfx_msg_t msg;
    for (;;) {
        gfx_poll(&msg);
        gfx_handle(&msg);
        if (msg.message == GFX_EV_QUIT)
            break;
        switch (msg.message) {
        case GFX_EV_MOUSEMOVE:
            on_mse_move(win, tty);
            break;
        case GFX_EV_BTNUP:
            on_mse_up(win, tty, msg.param1);
            break;
        case GFX_EV_BTNDOWN:
            on_mse_down(win, tty, msg.param1);
            break;
        case GFX_EV_KEYPRESS:
            terminal_key(tty, msg.param1, msg.gfx->seat->kdb_status);
            break;
        case GFX_EV_MOUSEWHEEL:
            terminal_scroll(tty, (int)msg.param1);
            break;
        case GFX_EV_RESIZE:
            terminal_resize(tty, win);
            break;
        case GFX_EV_TIMER:
            if (terminal_redraw(tty)) {
                terminal_paint(tty);
                gfx_flip(win, NULL);
            }
            break;
        }

    }


    gfx_unmap(win);
    gfx_destroy(win);
    terminal_destroy(tty);
    return 0;
}
