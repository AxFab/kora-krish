#include "krish.h"
#include <kora/gfx.h>
#include "threads.h"
#include "llist.h"
#include <assert.h>
#include <stdio.h>


#define _sR(v)  (((v) & 0xFF) << 16)
#define _sG(v)  (((v) & 0xFF) << 8)
#define _sB(v)  ((v) & 0xFF)

#define _gR(v)  (((v) >> 16) & 0xFF)
#define _gG(v)  (((v) >> 8) & 0xFF)
#define _gB(v)  ((v) & 0xFF)

#define COLOR_NEG(v)  ( _sR(256-_gR(v)) | _sG(256-_gG(v)) | _sB(256-_gB(v)) )

typedef struct term_cell {
    uint32_t fcolor;
    uint32_t bcolor;
    int col, row;
    llnode_t node;
    int len, cap;
    char *pen;
    char text[0];
} term_cell_t;

typedef struct term_line {
    int row;
    int flags;
    llhead_t cells;
} term_line_t;

typedef struct term_buffer {
    int top, size;
    term_cell_t *last;
    term_line_t lines[65536];
} term_buffer_t;

struct termio {
    uint32_t colors[16];
    int tab_size;
    int scroll;
    int start_row;
    int start_col;
    int end_row;
    int end_col;
    bool resize;
    bool auto_scroll;
    bool *invals;
    term_buffer_t buf_lines;
    term_buffer_t buf_input;
    mtx_t mtx;
    job_t *fjob;
    void(*readline)(termio_t *, const char *);

    gfx_t *win;
    const font_bmp_t *font;
    int cols, rows;
    bool invalid;
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static term_cell_t *terminal_create_cell(termio_t *tty)
{
    term_cell_t *cell = calloc(8192, 1);
    cell->cap = 8192 - sizeof(term_cell_t);
    cell->pen = cell->text;
    cell->fcolor = tty->colors[7];
    cell->bcolor = tty->colors[0];
    return cell;
}

static void terminal_free_line(term_buffer_t *buffer, int idx)
{
    idx = idx % buffer->size;
    while (buffer->lines[idx].cells.count_) {
        term_cell_t *cell = ll_take(&buffer->lines[idx].cells, term_cell_t, node);
        free(cell);
    }
}

static void terminal_free_buffer(termio_t *tty, term_buffer_t *buffer)
{
    int i;
    for (i = 0; i < buffer->size; ++i)
        terminal_free_line(buffer, i);
    buffer->last->row = 0;
    buffer->last->col = 0;
    buffer->last->fcolor = tty->colors[7];
    buffer->last->bcolor = tty->colors[0];
}

termio_t *terminal_create(void(*readline)(termio_t *, const char *))
{
    termio_t *tty = calloc(sizeof(termio_t), 1);
    tty->tab_size = 8;
    tty->resize = true;
    tty->readline = readline;
    tty->start_row = -1;
    tty->end_row = -1;
    tty->font = &font_7x13;

    tty->colors[0] = 0x101010;
    tty->colors[1] = 0xa61010;
    tty->colors[2] = 0x10a610;
    tty->colors[3] = 0xa6a610;
    tty->colors[4] = 0x1010a6;
    tty->colors[5] = 0xa610a6;
    tty->colors[6] = 0x10a6a6;
    tty->colors[7] = 0xa6a6a6;
    tty->colors[8] = 0x606060;
    tty->colors[9] = 0xf06060;
    tty->colors[10] = 0x60f060;
    tty->colors[11] = 0xf0f060;
    tty->colors[12] = 0x6060f0;
    tty->colors[13] = 0xf060f0;
    tty->colors[14] = 0x60f0f0;
    tty->colors[15] = 0xf0f0f0;

    tty->buf_lines.size = 65536;
    tty->buf_lines.last = terminal_create_cell(tty);

    tty->buf_input.size = 64;
    tty->buf_input.last = terminal_create_cell(tty);
    mtx_init(&tty->mtx, mtx_plain);
    return tty;
}

void terminal_destroy(termio_t *tty)
{
    int i;
    if (tty->invals)
        free(tty->invals);
    for (i = 0; i < tty->buf_lines.size; ++i)
        terminal_free_line(&tty->buf_lines, i);
    for (i = 0; i < tty->buf_input.size; ++i)
        terminal_free_line(&tty->buf_input, i);

    free(tty->buf_lines.last);
    free(tty->buf_input.last);
    free(tty);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void terminal_font_size(termio_t *tty, int *w, int *h)
{
    *w = tty->font->dispx;
    *h = tty->font->dispy;
}

void terminal_cell(termio_t *tty, term_buffer_t *buffer)
{
    term_cell_t *cell = buffer->last;
    *cell->pen = '\0';
    int lg = cell->pen - cell->text;
    if (lg != 0) {
        lg++;
        term_cell_t *copy = calloc(sizeof(term_cell_t) + lg, 1);
        memcpy(copy, cell, sizeof(term_cell_t) + lg);
        copy->cap = lg;
        int idx = copy->row % buffer->size;
        buffer->lines[idx].flags = 0;
        buffer->lines[idx].row = copy->row;
        ll_append(&buffer->lines[idx].cells, &copy->node);
    }
    cell->col += cell->len;
    cell->pen = cell->text;
    cell->len = 0;
}

void terminal_inval_rows(termio_t *tty, int from, int to)
{
    // printf("INVAL: %d -> %d\n", from, to);
    int rows = tty->rows;
    if (tty->invals == NULL)
        tty->invals = calloc(sizeof(bool), rows);
    from = MAX(0, from);
    to = MIN(to, rows);
    for (; from <= to; ++from)
        tty->invals[from] = true;
    tty->invalid = true;
}

void terminal_ansi_graphics(termio_t *tty, term_cell_t *cell, int *values, int sp)
{
    uint32_t tmp;
    for (; sp > 0; values++, sp--) {
        if (values[0] == 0) { // Reset
            cell->fcolor = tty->colors[7];
            cell->bcolor = tty->colors[0];
        } else if (values[0] == 1) { // Bold
        } else if (values[0] == 2) { // Faint
        } else if (values[0] == 3) { // Italic
        } else if (values[0] == 4) { // Underline
        } else if (values[0] == 5) { // Slow blink (-150/min)
        } else if (values[0] == 6) { // Fast blink (+150/min)
        } else if (values[0] == 7) { // Reverse
            tmp = cell->fcolor;
            cell->fcolor = cell->bcolor;
            cell->bcolor = tmp;
        } else if (values[0] == 8) { // Conceal
        } else if (values[0] == 9) { // Crossed-out
        } else if (values[0] < 21) { // Select font
        } else if (values[0] < 30) {
        } else if (values[0] < 38) { // Foreground
            cell->fcolor = tty->colors[values[0] - 30];
        } else if (values[0] == 38) {
        } else if (values[0] == 39) {
            cell->fcolor = tty->colors[7];
        } else if (values[0] < 48) { // Background
            cell->bcolor = tty->colors[values[0] - 40];
        } else if (values[0] == 48) {
        } else if (values[0] == 49) {
            cell->bcolor = tty->colors[0];
        } else if (values[0] < 90) {
        } else if (values[0] < 98) { // Foreground
            cell->fcolor = tty->colors[values[0] - 90 + 8];
        }
    }
}

int terminal_ansi_sequence(termio_t *tty, term_cell_t *cell, const char *buf, int len)
{
    char *r = (char *)&buf[1];
    int values[15];
    int sp = 0;
    do {
        ++r;
        values[sp++] = strtol(r, &r, 10);
    } while (*r == ';' && sp < 15);

    if (*r == ';')
        return 2;
    else if (*r == 'A') // Cursor up (n)
        cell->row -= sp > 0 ? values[0] : 1;
    else if (*r == 'B') // Cursor down(n)
        cell->row += sp > 0 ? values[0] : 1;
    else if (*r == 'C') // Cursor forward (n)
        cell->col += sp > 0 ? values[0] : 1;
    else if (*r == 'D') // Cursor backward (n)
        cell->col = MAX(0, cell->col - (sp > 0 ? values[0] : 1));
    else if (*r == 'E') { // Cursor next line (n:1)
        cell->col = 0;
        cell->row += sp > 0 ? values[0] : 1;
    } else if (*r == 'F') { // Cursor previous line (n:1)
        cell->col = 0;
        cell->row -= sp > 0 ? values[0] : 1;
    } else if (*r == 'H' && sp >= 2) { // Cursor position (n;n)
        cell->col = values[1] - 1;
        cell->row = values[0] + tty->scroll;
    } else if (*r == 'J') {
        // Erase in display (0: cur to end, 1:start to cur, 2:screen, 3:screen + buffer)
    } else if (*r == 'K') {
        // Erase in line (0: cur to end, 1:start to cur, 2:all)
    } else if (*r == 'S') {
        // Scroll page up
    } else if (*r == 'T') {
        // Scroll page down
    } else if (*r == 'm' && sp > 0) {
        // Set graphics
        terminal_ansi_graphics(tty, cell, values, sp);
    }

    return (r - buf) + 1;
}

void terminal_control(termio_t *tty, term_buffer_t *buffer, char ctrl)
{
    terminal_cell(tty, buffer);
    switch (ctrl) {
    case '\t':
        buffer->last->col += tty->tab_size - (buffer->last->col % tty->tab_size);
        break;
    case '\r':
        buffer->last->col = 0;
        break;
    case '\n':
        buffer->last->col = 0;
        buffer->last->row++;
        terminal_free_line(buffer, buffer->last->row);
        break;
    default:
        break;
    }
}

void terminal_write_chars(termio_t *tty, term_buffer_t *buffer, const unsigned char *buf, size_t len, int scroll)
{
    int from = buffer->last->row - scroll;
    while (len > 0) {
        int lg = uclen(buf, len);
        if (lg <= 0) {
            len--;
            buf++;
            continue;
        }

        if (lg > 1 || *buf >= 0x20) {
            if (buffer->last->len + lg > buffer->last->cap)
                terminal_cell(tty, buffer);
            while (lg-- > 0) {
                *(buffer->last->pen++) = *(buf++);
                len--;
            }

            buffer->last->len++;
            continue;
        }

        // Control characters
        if (*buf == '\033') {
            terminal_cell(tty, buffer);
            if (len < 3 || buf[1] != '[')
                break;
            lg = terminal_ansi_sequence(tty, buffer->last, buf, len);
            len -= lg;
            buf += lg;
        } else {
            terminal_control(tty, buffer, *buf);
            len--;
            buf++;
        }
    }

    int idx = buffer->last->row % buffer->size;
    buffer->lines[idx].flags = 0;
    terminal_inval_rows(tty, from, buffer->last->row - scroll);
}

void terminal_backspace(termio_t *tty, term_buffer_t *buffer)
{
    if (buffer->last->len <= 0)
        return;
    buffer->last->len--;
    buffer->last->pen--;
}

void terminal_write_into(termio_t *tty, term_buffer_t *src, term_buffer_t *dest)
{
    int i;
    terminal_cell(tty, src);
    terminal_cell(tty, dest);
    int from = dest->last->row;
    for (i = 0; i < src->size; ++i) {
        if (src->lines[i].cells.count_ == 0)
            continue;
        if (i != 0)
            dest->last->col = 0;
        term_cell_t *cell = NULL;
        terminal_free_line(dest, dest->last->row + i);
        dest->lines[(dest->last->row + i) % dest->size].row = dest->last->row + i;
        while (src->lines[i].cells.count_) {
            cell = ll_take(&src->lines[i].cells, term_cell_t, node);
            cell->row = dest->last->row + i;
            cell->col += dest->last->col;
            int idx = cell->row % dest->size;
            ll_push_back(&dest->lines[idx].cells, &cell->node);
            dest->lines[idx].flags = 0;
            dest->last->row = cell->row;
        }
        dest->last->col = cell->col + cell->len;
    }
    src->last->row = 0;
    src->last->col = 0;
    terminal_inval_rows(tty, from - tty->scroll, dest->last->row - tty->scroll);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static int terminal_cell_selected(termio_t *tty, int idx, term_cell_t *cell)
{
    if (tty->start_row > idx || tty->end_row < idx)
        return 0;
    else if (tty->start_row == idx && tty->end_row == idx) {
        if (tty->start_col >= cell->col + cell->len)
            return 0;
        else if (tty->end_col <= cell->col)
            return 0;
        else if (tty->start_col <= cell->col)
            return 2;
        else if (tty->end_col <= cell->col + cell->len)
            return 4;
        return 3;
    } else if (tty->start_row == idx) {
        if (tty->start_col >= cell->col + cell->len)
            return 0;
        else if (tty->start_col > cell->col)
            return 3;
        return 1;
    } else if (tty->end_row == idx) {
        if (tty->end_col <= cell->col)
            return 0;
        else if (tty->end_col <= cell->col + cell->len)
            return 2;
        return 1;
    }
    return 1;
}

void display_cell(termio_t *tty, int row, int col, const char *text, int len, uint32_t fg, uint32_t bg)
{
    gfx_t *gfx = tty->win;
    const font_bmp_t *font = tty->font;
    int i, s = 0;
    for (i = 0; i < len; ++i) {
        if ((col + i - s) * font->dispx > gfx->width)
            break;
        if (text[i] >= 0x20)
            gfx_glyph(gfx, font, text[i], fg, bg, (col + i - s) * font->dispx, row * font->dispy);
        else if (text[i] < 0) {
            uchar_t unicode;
            int ln = mbtouc(&unicode, &text[i], len);
            if (ln < 1)
                continue;
            gfx_glyph(gfx, font, 0x7F, fg, bg, (col + i - s) * font->dispx, row * font->dispy);
            s += ln - 1;
            i += ln - 1;
        }
    }
}

static inline void DISPLAY_CELL(termio_t *tty, int row, term_cell_t *cell)
{
    int ds, de;
    int selected = terminal_cell_selected(tty, row, cell);
    switch (selected) {
    case 0:
        display_cell(tty, row, cell->col, cell->text, cell->len, cell->fcolor, cell->bcolor);
        break;
    case 1:
        display_cell(tty, row, cell->col, cell->text, cell->len, COLOR_NEG(cell->fcolor), COLOR_NEG(cell->bcolor));
        break;
    case 2: // Only start
        ds = tty->end_col - cell->col;
        display_cell(tty, row, cell->col, cell->text, ds, COLOR_NEG(cell->fcolor), COLOR_NEG(cell->bcolor));
        display_cell(tty, row, cell->col + ds, &cell->text[ds], cell->len - ds, cell->fcolor, cell->bcolor);
        break;
    case 3: // Only end
        de = tty->start_col - cell->col;
        display_cell(tty, row, cell->col, cell->text, de, cell->fcolor, cell->bcolor);
        display_cell(tty, row, cell->col + de, &cell->text[de], cell->len - de, COLOR_NEG(cell->fcolor), COLOR_NEG(cell->bcolor));
        break;
    case 4: // Only middle
        ds = tty->start_col - cell->col;
        de = tty->end_col - cell->col;
        display_cell(tty, row, cell->col, cell->text, ds, cell->fcolor, cell->bcolor);
        display_cell(tty, row, cell->col + ds, &cell->text[ds], de - ds, COLOR_NEG(cell->fcolor), COLOR_NEG(cell->bcolor));
        display_cell(tty, row, cell->col + de, &cell->text[de], cell->len - de, cell->fcolor, cell->bcolor);
        break;
    }
}

void terminal_write(termio_t *tty, const unsigned char *buf, size_t len)
{
    mtx_lock(&tty->mtx);
    terminal_write_chars(tty, &tty->buf_lines, buf, len, tty->scroll);
    mtx_unlock(&tty->mtx);
}

void terminal_scroll(termio_t *tty, int speed)
{
    mtx_lock(&tty->mtx);
    int last = tty->buf_lines.last->row;
    last++;
    int total = last + tty->buf_input.last->row + 1;
    tty->auto_scroll = false;
    // printf("Scrolling: %d (%d + %d)\n", last, tty->scroll, speed);
    tty->scroll = MIN(last - 1, MAX(0, tty->scroll - speed));
    terminal_inval_all(tty);
    mtx_unlock(&tty->mtx);
}

bool terminal_redraw(termio_t *tty)
{
    return tty->invalid;
}

void terminal_paint(termio_t *tty)
{
    int i;
    mtx_lock(&tty->mtx);
    tty->invalid = false;
    if (tty->invals == NULL)
        tty->resize = true;
    int rows = tty->rows;
    int last = tty->buf_lines.last->row;
    if (tty->buf_lines.last->len > 0)
        last++;
    int total = last + tty->buf_input.last->row + 1;
    if (total - tty->scroll > rows && tty->auto_scroll) { // TODO -- and auto scroll
        tty->resize = true;
        if (tty->invals != NULL) {
            free(tty->invals);
            tty->invals = NULL;
        }
        tty->scroll = total - rows;
    }

    if (tty->resize == true)
        gfx_clear(tty->win, tty->colors[0]);

    // Debug
    gfx_t *win = tty->win;
    gfx_rect(win, win->width - 3, 0, 3, win->height, 0x00f2c2);

    int pl = tty->buf_lines.last->len > 0 ? 0 : 1;
    for (i = 0; i < rows; ++i) {
        term_buffer_t *buffer = &tty->buf_lines;
        int row = i + tty->scroll;
        if (row >= tty->buf_lines.last->row) {
            row -= tty->buf_lines.last->row;
            buffer = &tty->buf_input;
            if (row >= tty->buf_input.last->row)
                tty->auto_scroll = true;
        }

        int idx = row % buffer->size;
        if (!tty->resize && !tty->invals[i])
            continue;
        if (buffer->lines[idx].row != row) {
            if (!tty->resize && tty->invals[i])
                gfx_rect(tty->win, 0, i * tty->font->dispy, tty->win->width, tty->font->dispy, tty->colors[0]);
            continue;
        }

        // Draw lines
        gfx_rect(tty->win, 0, i * tty->font->dispy, tty->win->width, tty->font->dispy, tty->colors[0]);
        term_cell_t *cell;
        for ll_each(&buffer->lines[idx].cells, cell, term_cell_t, node)
            DISPLAY_CELL(tty, i, cell);
        if (buffer->last->row == idx)
            DISPLAY_CELL(tty, i, buffer->last);

        if (tty->invals != NULL && tty->invals[i] == true) {
            int dy = tty->font->dispy;
            gfx_rect(win, win->width - 3, i * dy, 3, dy, 0xf20032);
            tty->invals[i] = false;
        }
    }

    tty->resize = false;
    mtx_unlock(&tty->mtx);
}


const int __clipboard_size = 8192;
char __clipboard_buffer[8192];

void terminal_key(termio_t *tty, uchar_t unicode, int status)
{
    int i;
    mtx_lock(&tty->mtx);
    if (status & KEY_STATUS_CTRL && status != 0xC) {
        switch (unicode) {
        case 'd':
            if (tty->fjob)
                job_close_stdin(tty->fjob);
            break;
        case 'l':
            terminal_scroll(tty, -200000000);
            terminal_inval_all(tty);
            break;
        case 'z':
            if (tty->fjob) {
                tty->fjob = job_background(tty->fjob);
                if (tty->fjob == NULL)
                    shell_prompt(tty);
            }
            break;
        case 'c':
            terminal_copy(tty, __clipboard_buffer, __clipboard_size);
            int len = strlen(__clipboard_buffer);
            if (len > 0)
                clipboard_copy(__clipboard_buffer, len + 1);
            break;
        case 'v':
            i = clipboard_paste(__clipboard_buffer, __clipboard_size);
            if (i > 0)
                terminal_paste(tty, __clipboard_buffer, i);
            break;
        }
        mtx_unlock(&tty->mtx);
        return;
    }

    if (unicode <= 0) {
        mtx_unlock(&tty->mtx);
        return;
    } else if (unicode < 0x20) {
        int st = tty->buf_lines.last->row - tty->scroll;
        switch (unicode) {
        case 8:
            terminal_backspace(tty, &tty->buf_input);
            break;
        case '\n':
            *tty->buf_input.last->pen = '\0';
            char *dup = strdup(tty->buf_input.last->text);
            terminal_write_into(tty, &tty->buf_input, &tty->buf_lines);
            terminal_control(tty, &tty->buf_lines, '\n');
            mtx_unlock(&tty->mtx);
            if (tty->fjob)
                job_puts(tty->fjob, dup);
            else
                tty->readline(tty, dup);
            mtx_lock(&tty->mtx);
            free(dup);
            break;
        default:
            break;
        }
        int ed = tty->buf_input.last->row - (tty->scroll - tty->buf_lines.last->row);
        terminal_inval_rows(tty, st, ed);
    } else if (unicode < 0x80) {
        terminal_write_chars(tty, &tty->buf_input, (char*)&unicode, 1, tty->scroll - tty->buf_lines.last->row);
    } else {
        char buf[UC_LEN_MAX];
        int lg = uctomb(buf, unicode);
        if (lg <= 1) {
            mtx_unlock(&tty->mtx);
            return;
        }
        terminal_write_chars(tty, &tty->buf_input, buf, lg, tty->scroll - tty->buf_lines.last->row);
    }
    mtx_unlock(&tty->mtx);
}

void terminal_prompt(termio_t *tty, const char *buf)
{
    terminal_free_buffer(tty, &tty->buf_input);
    mtx_lock(&tty->mtx);
    terminal_write_chars(tty, &tty->buf_input, buf, strlen(buf), tty->scroll - tty->buf_lines.last->row);
    terminal_cell(tty, &tty->buf_input);
    mtx_unlock(&tty->mtx);
}

void terminal_puts(termio_t *tty, const char *buf)
{
    mtx_lock(&tty->mtx);
    terminal_write_chars(tty, &tty->buf_lines, buf, strlen(buf), tty->scroll);
    mtx_unlock(&tty->mtx);
}

void terminal_select(termio_t *tty, int start_row, int start_col, int end_row, int end_col)
{
    mtx_lock(&tty->mtx);
    terminal_inval_rows(tty, tty->start_row, tty->end_row);
    if (start_row == end_row && start_col == end_col) {
        tty->start_row = -1;
        tty->start_col = 0;
        tty->end_row = -1;
        tty->end_col = 0;
    } else if (start_row < end_row || (start_row == end_row && start_col < end_col)) {
        tty->start_row = start_row;
        tty->start_col = start_col;
        tty->end_row = end_row;
        tty->end_col = end_col;
    } else {
        tty->start_row = end_row;
        tty->start_col = end_col;
        tty->end_row = start_row;
        tty->end_col = start_col;
    }
    terminal_inval_rows(tty, tty->start_row, tty->end_row);
    mtx_unlock(&tty->mtx);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void terminal_push_job(termio_t *tty, job_t *job)
{
    tty->fjob = job;
}

void terminal_pop_job(termio_t *tty, job_t *job)
{
    if (tty->fjob == job)
        tty->fjob = NULL;
}

job_t *terminal_job(termio_t *tty)
{
    return tty->fjob;
}

void terminal_resize(termio_t *tty, gfx_t *win)
{
    mtx_lock(&tty->mtx);
    tty->win = win;
    if (tty->font != NULL) {
        tty->rows = win->height / tty->font->dispy;
        tty->cols = win->width / tty->font->dispx;
    }
    terminal_inval_all(tty);
    mtx_unlock(&tty->mtx);
}

void terminal_inval_all(termio_t *tty)
{
    tty->resize = true;
    if (tty->invals)
        free(tty->invals);
    tty->invals = NULL;
    tty->invalid = true;
}

int terminal_copy(termio_t *tty, char *buf, int lg)
{
    int st = lg;
    int i, ds, de;
    term_cell_t *cell;
    mtx_lock(&tty->mtx);
    // TODO - Add parameters for color changes
    for (i = tty->start_row; i <= tty->end_row; ++i) {
        int row = i + tty->scroll;
        if (row >= tty->buf_lines.last->row)
            break;
        int idx = row % tty->buf_lines.size;
        if (st != lg) {
            lg--;
            *(buf++) = '\n';
        }
        term_line_t *line = &tty->buf_lines.lines[idx];
        for ll_each(&line->cells, cell, term_cell_t, node) {
            int selected = terminal_cell_selected(tty, i, cell);
            if (selected == 0)
                continue;
            switch (selected) {
            case 1:
                memcpy(buf, cell->text, cell->len);
                buf += cell->len;
                lg -= cell->len;
                break;
            case 2: // Only start
                ds = tty->end_col - cell->col;
                memcpy(buf, cell->text, ds);
                buf += ds;
                lg -= ds;
                break;
            case 3: // Only end
                de = tty->start_col - cell->col;
                memcpy(buf, &cell->text[de], cell->len - de);
                buf += cell->len - de;
                lg -= cell->len - de;
                break;
            case 4: // Only middle
                ds = tty->start_col - cell->col;
                de = tty->end_col - cell->col;
                memcpy(buf, &cell->text[ds], de - ds);
                buf += de - ds;
                lg -= de - ds;
                break;
            }
        }
    }
    *buf = '\0';
    mtx_unlock(&tty->mtx);
    return st - lg;
}

int terminal_paste(termio_t *tty, const char *buf, int len)
{
    mtx_lock(&tty->mtx);
    terminal_write_chars(tty, &tty->buf_input, buf, len, tty->scroll - tty->buf_lines.last->row);
    mtx_unlock(&tty->mtx);
    return 0;
}



