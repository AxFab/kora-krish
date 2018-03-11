#include <X11/Xlib.h>
#include "shell.h"
#include "kdb/keys.h"



struct frame_info {
  Display *d_;
  Window w_;
  GC gc_;
  int s_;
  int width_;
  int height_;
  int max_row_;
  int max_col_;
};

struct frame_info fb;

char input_buf [4096] = {0};
int input_pen = 0;
int input_orig = 0;
int output_count = 0;

void Display_char(struct term_line *line, int glyph, int row, int col)
{
  ++output_count;
  XSetForeground(fb.d_, fb.gc_, line->txColor_);
  XDrawString(fb.d_, fb.w_, fb.gc_, 2 + col * 6, 12 * row, (char*)&glyph, 1);
}

void Display_paint()
{
  int i, pen;
  struct term_line* line;
  fb.gc_ = XDefaultGC(fb.d_, fb.s_);

  XSetForeground(fb.d_, fb.gc_, 0x323232);
  XFillRectangle(fb.d_, fb.w_, fb.gc_, 0, 0, fb.width_, fb.height_);

  line = Termio_top();
  for (i = 1; i <= fb.max_row_; ++i) {
    output_count = 0;
    pen = Termio_push_line(line, i);
    if (pen == 0) {
      if (output_count > 0)
        ++i;
      break;
    }
    line = line->next_;
  }

  if (input_pen > 0) {
    XDrawString(fb.d_, fb.w_, fb.gc_, 2, 12 * i, input_buf, input_pen);
  }
  // XDrawString(fb.d_, fb.w_, fb.gc_, 2, 12 * 3, "fab@axsmk:~/> uname -a", 22);
  // XDrawString(fb.d_, fb.w_, fb.gc_, 2, 12 * 4, "Smoke axsmk 1.0-x86 SmkOS-1.0-x86-2017.May.09 i386 pc SmokeOS", 61);
  // XDrawString(fb.d_, fb.w_, fb.gc_, 2, 12 * 5, "fab@axsmk:~/> ls -lh", 20);
  // XDrawString(fb.d_, fb.w_, fb.gc_, 2, 12 * 6, "drwxr-xr-x 1 fab 4.00K Apr 23 13:05 smokeOs", 43);
  // XDrawString(fb.d_, fb.w_, fb.gc_, 2, 12 * 7, "-rw-r--r-- 1 fab 12.4K Nov 04  2016 Intel-x86.pdf", 49);
  // XDrawString(fb.d_, fb.w_, fb.gc_, 2, 12 * 8, "-rw-r--r-- 1 fab 12.4K Nov 04  2016 Intel-x86-dev.pdf", 53);
  // XDrawString(fb.d_, fb.w_, fb.gc_, 2, 12 * 9, "-rwxr-xr-x 1 fab 2.13K Feb 23 17:58 helper.sh", 45);
  // XDrawString(fb.d_, fb.w_, fb.gc_, 2, 12 * 10, "fab@axsmk:~/> ", 14);
  // if (input_pen > 0) {
  //   XDrawString(fb.d_, fb.w_, fb.gc_, 2, 12 * 11, input_buf, input_pen);
  // }
}

int Display_init()
{
  fb.height_ = 400;
  fb.width_ = fb.height_ / 10 * 16;
  fb.max_row_ = fb.height_ / 12;
  fb.max_col_ = fb.width_ / 6;
  fb.d_ = XOpenDisplay(NULL);
  if (fb.d_ == NULL) {
    fprintf(stderr, "Cannot open display\n");
    return -1;
  }
  fb.s_ = DefaultScreen(fb.d_);
  fb.w_ = XCreateSimpleWindow(fb.d_, RootWindow(fb.d_, fb.s_), 10, 10, fb.width_, fb.height_, 1,
                          BlackPixel(fb.d_, fb.s_), WhitePixel(fb.d_, fb.s_));
  XSelectInput(fb.d_, fb.w_, ExposureMask | KeyPressMask | KeyReleaseMask);
  XMapWindow(fb.d_, fb.w_);
  fb.gc_ = XDefaultGC(fb.d_, fb.s_);
  Termio_init(fb.max_row_, fb.max_col_, &Display_char);
  return 0;
}

void Display_close()
{
  XCloseDisplay(fb.d_);
}


void Display_redraw() {
  XExposeEvent paint;
  memset(&paint, 0, sizeof(XExposeEvent));
  paint.type = Expose;
  paint.send_event = True;
  paint.display = fb.d_;
  paint.window = fb.w_;
  paint.x = 0;
  paint.y = 0;
  paint.width = fb.width_;
  paint.height = fb.height_;
  paint.count = 0;
  XSendEvent(fb.d_, fb.w_, True, ExposureMask, (XEvent*)&paint);
}


void Display_loop()
{
  int character;
  XEvent e;
  XKeyEvent *ke = (XKeyEvent *)&e;
  Display_redraw();
  for (;;) {
    XNextEvent(fb.d_, &e);
    switch (e.type) {
      case Expose:
        // Display_clip(e.expo.x, e.expo.ye.expo.width, e.expo.height);
        Display_paint();
        // if (e.expo .count == 0)
        //   Display_swap();
        break;

      case KeyRelease:
        Keyboard_release(ke->keycode);
        break;

      case KeyPress:
        character = Keyboard_press(ke->keycode);
        if (character >= 0x20 && character < 0x80) {
          input_buf[input_pen] = character;
          input_buf[++input_pen] = '\0';
        } else if (character >= 0x80) {
          input_buf[input_pen] = 0x80;
          input_buf[++input_pen] = '\0';
        } else {
          switch (character) {
            case KEY_BACKSPACE:
              if (input_pen > input_orig)
                input_buf[--input_pen] = '\0';
              break;
            case KEY_UP:
              if (Keyboard_state() & 4) {
                Termio_scroll(-1);
              }
              // ELSE READ HISTORY PREV!
              break;
            case KEY_DOWN:
              if (Keyboard_state() & 4) {
                Termio_scroll(1);
              }
              // ELSE READ HISTORY PREV!
              break;
            case KEY_ENTER:
              Termio_write(input_buf, input_pen);
              Termio_write("\n", 1);
              Display_paint();
              return;
          }
        }
        Display_redraw();
        break;
    }
  }
}

TCHAR *Display_promt(TCHAR *promt) {
  _tcscpy(input_buf, promt);
  printf("Prompt %s\n", input_buf);
  input_orig = _tcslen(input_buf);
  input_pen = input_orig;
  Display_loop();
  return &input_buf[input_orig];
}

