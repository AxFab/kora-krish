#include <gfx.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int window(int service, int width, int height, int flags);

void clipboard_copy(const char *buf, int len)
{
}

int clipboard_paste(char *buf, int len)
{
    return 0;
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

const char *pipe_name = "";

gfx_t *gfx_create_window(void *ctx, int width, int height, int flag)
{
    gfx_t *gfx = calloc(sizeof(gfx_t), 1);
    gfx->width = width;
    gfx->height = height;
    gfx->fd = window(0, width, height, 0);
    if (gfx->fd == -1) {
        free(gfx);
        return NULL;
    }

    return gfx;
}

void gfx_destroy(gfx_t *gfx)
{
    gfx_unmap(gfx);
    close(gfx->fd);
    free(gfx);
}


int gfx_map(gfx_t *gfx)
{
    if (gfx->pixels != NULL)
        return 0;
    gfx->pitch = ALIGN_UP(gfx->width * 4, 4);
    gfx->pixels = mmap(NULL, 16 * _Mib_, 0x10002, 0, gfx->fd, 0);
    return 0;
}

int gfx_unmap(gfx_t *gfx)
{
    if (gfx->pixels == NULL)
        return 0;
    munmap(gfx->pixels, 16 * _Mib_);
    gfx->pixels = NULL;
    return 0;
}


void gfx_flip(gfx_t *gfx)
{
    if (gfx->pixels == NULL)
        return;
    fcntl(gfx->fd, 17/* FD_WFLIP */);
}

int gfx_loop(gfx_t *gfx, void *arg, gfx_handlers_t *handlers)
{
    int lx = 0, ly = 0, rx = 0, ry = 0;
    int key = 0, key2 = 0;

    gfx_seat_t seat;
    gfx_msg_t msg;
    memset(&seat, 0, sizeof(seat));
    if (gfx->fd < 0)
        return -1;

    handlers->expose(gfx, arg, &seat);
    gfx_flip(gfx);
    for (;;) {
        if (read(gfx->fd, (char*)&msg, sizeof(msg)) < sizeof(msg))
            continue;
        switch (msg.message) {
        case EV_QUIT:
            return 0;
        case EV_MOUSEMOVE:
            seat.mouse_x = msg.param1 & 0x7fff;
            seat.mouse_y = msg.param1 >> 16;
            if (handlers->mse_move)
                handlers->mse_move(gfx, arg, &seat);
            break;
        case EV_BUTTONDOWN:
            seat.btn_status |= msg.param1;
            if (handlers->mse_down)
                handlers->mse_down(gfx, arg, &seat, msg.param1);
            break;
        case EV_BUTTONUP:
            seat.btn_status &= ~msg.param1;
            if (handlers->mse_up)
                handlers->mse_up(gfx, arg, &seat, msg.param1);
            break;
        case EV_MOUSEWHEEL:
            if (handlers->mse_wheel)
                handlers->mse_wheel(gfx, arg, &seat, msg.param1);
            break;
        case EV_KEYDOWN:
            if (handlers->key_down)
                handlers->key_down(gfx, arg, &seat, msg.param2);
            break;
        case EV_KEYUP:
            if (handlers->key_up)
                handlers->key_up(gfx, arg, &seat, msg.param2);
            break;
        case EV_RESIZE:
            // gfx_unmap(gfx);
            gfx->width = msg.param1;
            gfx->height = msg.param2;
            gfx->pitch = ALIGN_UP(gfx->width * 4, 4);
            break;
        case EV_TIMER:
            if (handlers->repaint == NULL || handlers->repaint(gfx, arg, &seat)) {
                if (handlers->expose) {
                    handlers->expose(gfx, arg, &seat);
                    gfx_flip(gfx);
                }
            }
            break;
        default:
            break;
        }
    }
}





