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
    strcpy(host, "NUC");
    strcpy(user, "Fab");
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


void on_mse_up(gfx_t *gfx, termio_t *tty, gfx_seat_t *seat, int btn)
{
    int w, h;
    terminal_font_size(tty, &w, &h);
    if (btn == 1) {
        rx = seat->mouse_x / w;
        ry = seat->mouse_y / h;
        terminal_select(tty, ly, lx, ry, rx);
    }
}

void on_mse_down(gfx_t *gfx, termio_t *tty, gfx_seat_t *seat, int btn)
{
    if (btn == 1) {
        int w, h;
        terminal_font_size(tty, &w, &h);
        lx = rx = seat->mouse_x / w;
        ly = ry = seat->mouse_y / h;
    } else if (btn == 2) {

    } else if (btn == 3) {
        // TODO - Use non primary clipboard?
        int len = terminal_copy(tty, __clipboard_buffer, __clipboard_size);
        if (len > 0)
            terminal_paste(tty, __clipboard_buffer, len);
    }
}

void on_mse_move(gfx_t *gfx, termio_t *tty, gfx_seat_t *seat)
{
    if (seat->btn_status & 1) {
        int w, h;
        terminal_font_size(tty, &w, &h);
        rx = seat->mouse_x / w;
        ry = seat->mouse_y / h;
        terminal_select(tty, ly, lx, ry, rx);
    }
}

void on_mse_wheel(gfx_t *gfx, termio_t *tty, gfx_seat_t *seat, int disp)
{
    terminal_scroll(tty, disp);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */



int main(int argc, char const *argv[])
{
    job_init();
    termio_t *tty = terminal_create(on_readline);
    __tty = tty;
#ifndef main
    gfx_t *win = gfx_create_window(NULL, _16x10(480), 480, 0);
#else
    int fb0 = open("/dev/fb0", O_RDWR);
    int kdb = open("/dev/kdb", O_RDONLY);
    gfx_t *win = gfx_opend(fb0, kdb);
#endif
    terminal_resize(tty, win);

    gfx_map(win);
    terminal_puts(tty, "  Welcome on \033[96mKrish\033[0m, \033[90;43mv0.1.3\033[0m\n");
    shell_prompt(tty);

    gfx_msg_t msg;
    gfx_seat_t seat;
    memset(&seat, 0, sizeof(seat));
    gfx_keyboard_load(&seat);
    for (;;) {
        gfx_poll(win, &msg);
        gfx_handle(win, &msg, &seat);
        if (msg.message == GFX_EV_QUIT)
            break;
        switch (msg.message) {
        case GFX_EV_MOUSEMOVE:
            on_mse_move(win, tty, &seat);
            break;
        case GFX_EV_BTNUP:
            on_mse_up(win, tty, &seat, msg.param1);
            break;
        case GFX_EV_BTNDOWN:
            on_mse_down(win, tty, &seat, msg.param1);
            break;
        case GFX_EV_KEYPRESS:
            terminal_key(tty, msg.param1, seat.kdb_status);
            break;
        case GFX_EV_MOUSEWHEEL:
            terminal_scroll(tty, (int)msg.param1);
            break;
        case GFX_EV_RESIZE:
            terminal_resize(tty, win);
            break;
        case GFX_EV_PAINT:
            terminal_paint(tty);
            gfx_flip(win);
            break;
        }

    }


    gfx_unmap(win);
    gfx_destroy(win);
    terminal_destroy(tty);
    return 0;
}
