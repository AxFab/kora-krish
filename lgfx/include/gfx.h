#ifndef __GFX_H
#define __GFX_H 1

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <mcrs.h>

#define _16x10(n)  ((n)/10*16)

#define PACK(decl) decl __attribute__((__packed__))

typedef struct gfx gfx_t;
typedef struct gfx_seat gfx_seat_t;
typedef struct gfx_handlers gfx_handlers_t;
typedef struct font_bmp font_bmp_t;

struct gfx {
    int width;
    int height;
    int pitch;
    long fd;
    union {
        uint8_t *pixels;
        uint32_t *pixels4;
    };
};

struct gfx_seat {
    int mouse_x;
    int mouse_y;
    int btn_status; // left, right, wheel, prev, next
    int kdb_status; // Shift, caps, ctrl, alt, home, num, defl !
};

struct font_bmp {
    const uint8_t *glyphs;
    char glyph_size;
    char width;
    char height;
    char dispx;
    char dispy;
};

struct gfx_handlers {
    bool(*repaint)(gfx_t *gfx, void *arg, gfx_seat_t *seat);
    void(*quit)(gfx_t *gfx, void *arg, gfx_seat_t *seat);
    void(*destroy)(gfx_t *gfx, void *arg, gfx_seat_t *seat);
    void(*expose)(gfx_t *gfx, void *arg, gfx_seat_t *seat);
    void(*resize)(gfx_t *gfx, void *arg);
    void(*key_up)(gfx_t *gfx, void *arg, gfx_seat_t *seat, int key);
    void(*key_down)(gfx_t *gfx, void *arg, gfx_seat_t *seat, int key);
    void(*mse_up)(gfx_t *gfx, void *arg, gfx_seat_t *seat, int btn);
    void(*mse_down)(gfx_t *gfx, void *arg, gfx_seat_t *seat, int btn);
    void(*mse_move)(gfx_t *gfx, void *arg, gfx_seat_t *seat);
    void(*mse_wheel)(gfx_t *gfx, void *arg, gfx_seat_t *seat, int disp);
};


void gfx_clear(gfx_t *gfx, uint32_t color);
void gfx_rect(gfx_t *gfx, int x, int y, int w, int h, uint32_t color);
void gfx_glyph(gfx_t *gfx, const font_bmp_t *font, uint32_t unicode, uint32_t fg, uint32_t bg, int x, int y);

gfx_t *gfx_create_window(void *ctx, int width, int height, int flags);

void gfx_destroy(gfx_t *gfx);
int gfx_map(gfx_t *gfx);
int gfx_unmap(gfx_t *gfx);
int gfx_loop(gfx_t *gfx, void *arg, gfx_handlers_t *handlers);

void clipboard_copy(const char *buf, int len);
int clipboard_paste(char *buf, int len);

int keyboard_down(int key, int *status, int *key2);
int keyboard_up(int key, int *status);


extern const font_bmp_t font_6x10;
extern const font_bmp_t font_8x15;
extern const font_bmp_t font_7x13;
extern const font_bmp_t font_6x9;
extern const font_bmp_t font_8x8;

typedef struct gfx_msg {
    int message;
    int param;
} gfx_msg_t;

#define EV_QUIT  0
#define EV_MOUSEMOVE  2
#define EV_BUTTONDOWN  3
#define EV_BUTTONUP  4
#define EV_MOUSEWHEEL  5
#define EV_KEYDOWN  6
#define EV_KEYUP  7
#define EV_TIMER  8
#define EV_DELAY  9


#define KEY_SHIFT_L  1
#define KEY_SHIFT_R  2
#define KEY_CAPS_LOCK  3
#define KEY_ALT_L  4
#define KEY_ALT_R  5
#define KEY_CTRL_L  6
#define KEY_CTRL_R  7
#define KEY_NUM_LOCK  8

#define KEY_RIGHT -4
#define KEY_LEFT -4
#define KEY_UP -4
#define KEY_DOWN -4
#define KEY_PAGE_UP -4
#define KEY_PAGE_DOWN -3
#define KEY_INSERT -2
#define KEY_DELETE -1
#define KEY_BREAK -1
#define KEY_END -1
#define KEY_HOME -1

#define KEY_F1 -5
#define KEY_F2 -5
#define KEY_F3 -5
#define KEY_F4 -5
#define KEY_F5 -5
#define KEY_F6 -5
#define KEY_F7 -5
#define KEY_F8 -5
#define KEY_F9 -5
#define KEY_F10 -5
#define KEY_F11 -5
#define KEY_F12 -5

#define KDB_SHIFT  1
#define KDB_CAPSLOCK  2
#define KDB_ALT  4
#define KDB_CTRL  8
#define KDB_HOME  16

#endif  /* __GFX_H */
