/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2018  <Fabien Bavent>
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
#ifndef _SRC_FIFO_H
#define _SRC_FIFO_H 1

#include <stddef.h>
#include <assert.h>

#ifndef MIN
# define MIN(a,b) ((a)<(b)?(a):(b))
# define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define FP_NOBLOCK (1 << 0)
#define FP_EOL (1 << 1)
#define FP_WR (1 << 2)

typedef struct fifo fifo_t;

// TODO Need this only for wpen_ -- Access by function!
struct fifo
{
  size_t rpen_;
  size_t wpen_;
  size_t avail_;
  size_t size_;
  void *buf_;
  // mutex_t mtx_;
  // void (*wait)(fifo_t*, int);
  // void (*awake)(fifo_t*, int);
};




/* Instanciate a new FIFO */
fifo_t *fifo_init(void *buf, size_t lg);
/* Look for a specific bytes in consumable data */
size_t fifo_indexof(fifo_t *fifo, char ch) ;
/* Read some bytes from the FIFO */
size_t fifo_out(fifo_t *fifo, void *buf, size_t lg, int flags);
/* Write some bytes on the FIFO */
size_t fifo_in(fifo_t *fifo, const void *buf, size_t lg, int flags);
/* Reinitialize the queue */
void fifo_reset(fifo_t *fifo);

#endif  /* _SRC_FIFO_H */
