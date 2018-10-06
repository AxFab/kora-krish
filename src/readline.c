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
#include "shell.h"

char cmd[PATH_MAX];

TCHAR *Rdline_read_stub()
{
  static int i = 0;
  switch (i++) {
  case 0:
    return "prog arg1 < ( bg-prg arg2 ^>&99 ) | aggreg > 12 &";
  case 1:
    return NULL; // _tcsdup("exit");
  case 2:
    return "jkfhgdfr56f4f 4v5fdg65rg86r4vr46 465rrgtgbtbg86b t4h t6htr6 564 + 456d4";
  case 3:
    return "stdout >45 >>46 ";
  case 4:
    return "fds 4 f56f4g fg4d65 df [f]d]gfds4ds65 546 'dfd dfd gdg'   fd dfs\"fdf \" dfsdfd s";
  default:
    return NULL;
  }
}

TCHAR *Rdline_read(TCHAR *promt)
{
  char *gt;
  return Rdline_read_stub();
  gt = fgets(cmd, PATH_MAX, stdin);
  if (gt == NULL || gt == (void*)EOF) {
    fprintf(stderr, "Unable to read user input: [%d] %s.\n",
        errno, strerror(errno));
    return NULL;
  }
  return gt;
}

