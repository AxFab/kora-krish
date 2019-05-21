#include "gfx.h"
#include <stdio.h>
#include <windows.h>
#include <tchar.h>

#define COLOR_REF(n)  ( (((n) & 0xFF0000) >> 16) | ((n) & 0xFF00) | (((n) & 0xFF) << 16) )

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void clipboard_copy(const char *buf, int len)
{
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(hMem), buf, len);
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

int clipboard_paste(char *buf, int len)
{
    OpenClipboard(0);
    HGLOBAL hMem = GetClipboardData(CF_TEXT);
    void *ptr = GlobalLock(hMem);
    int size = GlobalSize(hMem);
    if (size < len)
        memcpy(buf, ptr, size);
    else
        size = 0;
    GlobalUnlock(hMem);
    CloseClipboard();
    return size;
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static TCHAR szWindowClass[] = _T("Krish");
static TCHAR szTitle[] = _T("Krish");
static WNDCLASSEX wcex;
HINSTANCE appInstance = NULL;

gfx_t *__win32_gfx = NULL;
void *__win32_arg = NULL;
gfx_handlers_t *__win32_handlers = NULL;

LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    switch (uMsg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_SIZE:
        if (__win32_gfx) {
            gfx_unmap(__win32_gfx);
            RECT rect;
            if (GetClientRect((HWND)__win32_gfx->fd, &rect) == TRUE) {
                __win32_gfx->width = rect.right;
                __win32_gfx->height = rect.bottom;
            }

            if (__win32_handlers->resize)
                __win32_handlers->resize(__win32_gfx, __win32_arg);
        }
        break;
    default:
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void gfx_win32_init_()
{
    if (appInstance != NULL)
        return;
    appInstance = GetModuleHandle(NULL);
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = 0; // CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = appInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, _T("Call to RegisterClassEx failed."), _T("Win32 Error"), 0);
        exit(-1);
    }
}

static void gfx_painting(gfx_t *gfx, gfx_handlers_t *handlers, void *arg, gfx_seat_t *seat)
{
    HWND hwnd = (HWND)gfx->fd;
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    gfx_map(gfx);

    handlers->expose(gfx, arg, seat);

    // Copy buffer to device
    HBITMAP backbuffer = CreateBitmap(gfx->width, gfx->height, 1, 32, gfx->pixels);
    HDC backbuffDC = CreateCompatibleDC(hdc);
    SelectObject(backbuffDC, backbuffer);
    BitBlt(hdc, 0, 0, gfx->width, gfx->height, backbuffDC, 0, 0, SRCCOPY);

    DeleteObject(backbuffer);
    DeleteDC(backbuffDC);
    EndPaint(hwnd, &ps);
}


gfx_t *gfx_create_window(void *ctx, int width, int height, int flag)
{
    gfx_t *gfx = calloc(sizeof(gfx_t), 1);
    gfx->width = width;
    gfx->height = height;

    if (appInstance == NULL)
        gfx_win32_init_();
    gfx->fd = (int)CreateWindowEx(WS_EX_CLIENTEDGE, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, width + 16, height + 39, NULL, NULL, appInstance, NULL);
    if (gfx->fd == 0) {
        free(gfx);
        return NULL;
    }

    UINT timer;
    SetTimer((HWND)gfx->fd, (UINT_PTR)&timer, 25, NULL);
    return gfx;
}

void gfx_destroy(gfx_t *gfx)
{
    DestroyWindow((HWND)gfx->fd);
    if (gfx->pixels)
        _aligned_free(gfx->pixels);
    free(gfx);
}


int gfx_map(gfx_t *gfx)
{
    RECT rect;
    if (gfx->pixels != NULL)
        return 0;

    if (GetClientRect((HWND)gfx->fd, &rect) == FALSE)
        return -1;
    gfx->width = rect.right;
    gfx->height = rect.bottom;
    gfx->pitch = ALIGN_UP(gfx->width * 4, 4);
    gfx->pixels = _aligned_malloc(gfx->pitch * gfx->height, 1024 * 16);
    return 0;
}

int gfx_unmap(gfx_t *gfx)
{
    if (gfx->pixels != NULL) {
        _aligned_free(gfx->pixels);
        gfx->pixels = NULL;
    }
    return 0;
}

const char *pipe_name = "";

int gfx_loop(gfx_t *gfx, void *arg, gfx_handlers_t *handlers)
{
    int lx = 0, ly = 0, rx = 0, ry = 0;
    int key = 0, key2 = 0;
    __win32_gfx = gfx;
    __win32_arg = arg;
    __win32_handlers = handlers;

    gfx_seat_t seat;
    memset(&seat, 0, sizeof(seat));
    MSG msg;
    HWND hwnd = (HWND)gfx->fd;
    if (hwnd == 0)
        return -1;
    for (;;) {
        if (!GetMessage(&msg, hwnd, 0, 0))
            continue;
        switch (msg.message) {
        case 0:
            return 0;
        case WM_PAINT:
            if (handlers->expose)
                gfx_painting(gfx, handlers, arg, &seat);
            break;
        case WM_MOUSEMOVE:
            seat.mouse_x = msg.lParam & 0x7fff;
            seat.mouse_y = msg.lParam >> 16;
            if (handlers->mse_move)
                handlers->mse_move(gfx, arg, &seat);
            break;
        case WM_LBUTTONDOWN:
            seat.btn_status = msg.wParam;
            if (handlers->mse_down)
                handlers->mse_down(gfx, arg, &seat, 1);
            break;
        case WM_LBUTTONUP:
            seat.btn_status = msg.wParam;
            if (handlers->mse_up)
                handlers->mse_up(gfx, arg, &seat, 1);
            break;
        case WM_RBUTTONDOWN:
            seat.btn_status = msg.wParam;
            if (handlers->mse_down)
                handlers->mse_down(gfx, arg, &seat, 2);
            break;
        case WM_RBUTTONUP:
            seat.btn_status = msg.wParam;
            if (handlers->mse_up)
                handlers->mse_up(gfx, arg, &seat, 2);
            break;
        case WM_MBUTTONDOWN:
            seat.btn_status = msg.wParam;
            if (handlers->mse_down)
                handlers->mse_down(gfx, arg, &seat, 4);
            break;
        case WM_MBUTTONUP:
            seat.btn_status = msg.wParam;
            if (handlers->mse_up)
                handlers->mse_up(gfx, arg, &seat, 4);
            break;
        case WM_MOUSEWHEEL:
            if (handlers->mse_wheel)
                handlers->mse_wheel(gfx, arg, &seat, (signed short)(msg.wParam >> 16) / 40);
            break;
        case WM_KEYDOWN:
            if (handlers->key_down)
                handlers->key_down(gfx, arg, &seat, (msg.lParam >> 16) & 0x7F);
            break;
        case WM_KEYUP:
            if (handlers->key_up)
                handlers->key_up(gfx, arg, &seat, (msg.lParam >> 16) & 0x7F);
            break;
        case WM_TIMER:
            if (handlers->repaint == NULL || handlers->repaint(gfx, arg, &seat)) {
                RECT rect;
                rect.left = 0;
                rect.right = gfx->width;
                rect.top = 0;
                rect.bottom = gfx->height;
                InvalidateRect(hwnd, &rect, FALSE);
            }
            break;
        default:
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}





