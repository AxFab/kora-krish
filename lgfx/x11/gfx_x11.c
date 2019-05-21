#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <gfx.h>

void clipboard_copy(const char *buf, int len)
{
}

int clipboard_paste(char *buf, int len)
{
    return 0;
}

int __exec(char *name, int argc, char **argv, int fds[3])
{
    int i;
    char cmdline[4096];

    strncpy(cmdline, name, 4096);
    for (i = 0; i < argc; ++i) {
        strncat(cmdline, " ", 4096);
        strncat(cmdline, argv[i], 4096);
    }

    int pid = fork();
    if (pid != 0)
        return pid;

    exit(-1);
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

struct xwin {
    Display *display;
    Window window;
    GC gc;
    int screen;
    XImage *img;
};

gfx_t *gfx_create_window(void *ctx, int width, int height, int flags)
{
    struct xwin *wi = calloc(sizeof(struct xwin), 1);
    gfx_t *gfx = calloc(sizeof(gfx_t), 1);
    gfx->width = width;
    gfx->height = height;
    gfx->fd = (long)wi;
    wi->display = XOpenDisplay(NULL);
    if (wi->display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        free(wi);
        free(gfx);
        return NULL;
    }
    wi->screen = DefaultScreen(wi->display);
    wi->window = XCreateSimpleWindow(wi->display,
                                     RootWindow(wi->display, wi->screen), 10, 10,
                                     gfx->width, gfx->height, 1,
                                     BlackPixel(wi->display, wi->screen),
                                     WhitePixel(wi->display, wi->screen));
    XSelectInput(wi->display, wi->window, ExposureMask | KeyPressMask | KeyReleaseMask);
    XMapWindow(wi->display, wi->window);
    wi->gc = XDefaultGC(wi->display, wi->screen);
    return gfx;
}

void gfx_destroy(gfx_t *gfx)
{
    struct xwin *wi = (struct xwin *)gfx->fd;

    XCloseDisplay(wi->display);
}

int gfx_map(gfx_t *gfx)
{
    struct xwin *wi = (struct xwin *)gfx->fd;
    gfx->pitch = gfx->width * 4;
    gfx->pixels = calloc(gfx->pitch, gfx->height);

    wi->img = XCreateImage(wi->display, DirectColor, 4, XYBitmap, 0, gfx->pixels, gfx->width, gfx->height, 0, gfx->pitch);
    return 0;
}

int gfx_unmap(gfx_t *gfx)
{

    return 0;
}

static void gfx_flip(gfx_t *gfx)
{
    struct xwin *wi = (struct xwin *)gfx->fd;
    XPutImage(wi->display, wi->window, wi->gc, wi->img, 0, 0, 0, 0, gfx->width, gfx->height);
}

int gfx_loop(gfx_t *gfx, void *arg, gfx_handlers_t *handlers)
{
    struct xwin *wi = (struct xwin *)gfx->fd;
    XEvent e;
    XKeyEvent *ke = (XKeyEvent *)&e;
    for (;;) {
        XNextEvent(wi->display, &e);
        switch (e.type) {
        case Expose:
            if (handlers->expose) {
                handlers->expose(gfx, arg, NULL);
                gfx_flip(gfx);
            }
            // PAINT
            // COPY !
            //
            break;
        }
    }
    return 0;
}


