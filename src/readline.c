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

