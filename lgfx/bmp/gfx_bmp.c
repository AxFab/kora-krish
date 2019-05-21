#include <gfx.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

void clipboard_copy(const char *buf, int len)
{
}

int clipboard_paste(char *buf, int len)
{
    return 0;
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

const char *pipe_name = "win.in";

gfx_t *gfx_create_window(void *ctx, int width, int height, int flag)
{
    gfx_t *gfx = calloc(sizeof(gfx_t), 1);
    gfx->width = width;
    gfx->height = height;

    gfx->fd = open(pipe_name, O_RDONLY);
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
    gfx->pixels = calloc(gfx->pitch, gfx->height);
    return 0;
}

int gfx_unmap(gfx_t *gfx)
{
    if (gfx->pixels == NULL)
        return 0;
    free(gfx->pixels);
    gfx->pixels = NULL;
    return 0;
}

PACK(struct bmp_header {
    uint16_t magic;
    uint32_t fsize;
    uint32_t reserved;
    uint32_t offset;
    uint32_t hsize;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitsperpx;
    uint32_t compression;
    uint32_t isize;
    uint32_t xresol;
    uint32_t yresol;
    uint32_t a;
    uint32_t b;
});

int gfx_export_bmp24(const char *name, gfx_t *gfx)
{
    struct bmp_header head;
    int fd = open(name, O_WRONLY | O_CREAT, 0644);
    if (fd == -1)
        return -1;
    memset(&head, 0, sizeof(head));
    head.magic = 0x4d42;
    head.fsize = sizeof(head) + gfx->width * 3 * gfx->height;
    head.offset = sizeof(head);
    head.hsize = sizeof(head) - 0xE;
    head.width = gfx->width;
    head.height = gfx->height;
    head.planes = 1;
    head.bitsperpx = 24;
    head.isize = gfx->width * 3 * gfx->height;
    if (write(fd, &head, sizeof(head)) != sizeof(head)) {
        fprintf(stderr, "Error persisting image: %d\n", errno);
        return -1;
    }

    int x, y = gfx->height;
    while (y-- > 0) {
        for (x = 0; x < gfx->width; ++x)
            write(fd, &gfx->pixels4[y * gfx->width + x], 3);
    }
    close(fd);
    return 0;
}


void gfx_flip(gfx_t *gfx)
{
    if (gfx->pixels == NULL)
        return;
    gfx_export_bmp24("win.bmp", gfx);
}

void gfx_parse_(const char *buf, gfx_msg_t *msg)
{
    msg->param = 0;
    msg->message = 0;
    if (strncmp(buf, "KEYD ", 5) == 0) {
        msg->message = EV_KEYDOWN;
        sscanf(buf, "KEYD %x", &msg->param);
    } else if (strncmp(buf, "KEYU ", 5) == 0) {
        msg->message = EV_KEYUP;
        sscanf(buf, "KEYU %x", &msg->param);
    } else if (strncmp(buf, "QUIT", 4) == 0) {
        msg->message = EV_QUIT;
    } else if (strncmp(buf, "TIMER", 5) == 0) {
        msg->message = EV_TIMER;
    } else if (strncmp(buf, "DELAY", 5) == 0) {
        msg->message = EV_DELAY;
    } else {
        msg->message = -1;
    } 
}

int gfx_loop(gfx_t *gfx, void *arg, gfx_handlers_t *handlers)
{
    int lx = 0, ly = 0, rx = 0, ry = 0;
    int key = 0, key2 = 0;

    gfx_seat_t seat;
    gfx_msg_t msg;
    char buf[16];
    int cn, idx = 0;
    memset(&seat, 0, sizeof(seat));
    if (gfx->fd < 0)
        return -1;
    for (;;) {
        if (idx < 16)
            cn = read(gfx->fd, &buf[idx], 16 - idx);
        char *n = strchr(buf, '\n');
        if (n == NULL)
            exit(-5);
        *n = '\0';
        gfx_parse_(buf, &msg);
        printf("-%s\n", buf);
        idx = (++n) - buf;
        memmove(buf, n, 16 - idx);
        idx = 16 - idx;
        switch (msg.message) {
        case EV_QUIT:
            return 0;
        case EV_MOUSEMOVE:
            seat.mouse_x = msg.param & 0x7fff;
            seat.mouse_y = msg.param >> 16;
            if (handlers->mse_move)
                handlers->mse_move(gfx, arg, &seat);
            break;
        case EV_BUTTONDOWN:
            seat.btn_status |= msg.param;
            if (handlers->mse_down)
                handlers->mse_down(gfx, arg, &seat, msg.param);
            break;
        case EV_BUTTONUP:
            seat.btn_status &= ~msg.param;
            if (handlers->mse_up)
                handlers->mse_up(gfx, arg, &seat, msg.param);
            break;
        case EV_MOUSEWHEEL:
            if (handlers->mse_wheel)
                handlers->mse_wheel(gfx, arg, &seat, msg.param);
            break;
        case EV_KEYDOWN:
            if (handlers->key_down)
                handlers->key_down(gfx, arg, &seat, msg.param);
            break;
        case EV_KEYUP:
            if (handlers->key_up)
                handlers->key_up(gfx, arg, &seat, msg.param);
            break;
        case EV_TIMER:
            if (handlers->repaint == NULL || handlers->repaint(gfx, arg, &seat)) {
                if (handlers->expose) {
                    handlers->expose(gfx, arg, &seat);
                    gfx_flip(gfx);
                }
            }
            break;
        case EV_DELAY:
            usleep(500000);
            break;
        default:
            break;
        }
    }
}

