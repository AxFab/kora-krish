#include "shell.h"


char pwd[PATH_MAX];
char usr[PATH_MAX];

char *getcwd(char *buf, size_t size);
int getlogin_r(char *buf, size_t bufsize);


int Environ_refresh()
{
  // Find PWD
  if (getcwd(pwd, PATH_MAX) == NULL) {
    fprintf(stderr, "Unable to find working directory: [%d] %s.\n",
        errno, strerror(errno));
    return -1;
  }

  // Find USERNAME
  if (getlogin_r(usr, PATH_MAX)) {
    fprintf(stderr, "Unable to find username: [%d] %s.\n",
        errno, strerror(errno));
    return -1;
  }

  return 0;
}

void leave() {
  Shell_sweep();
  Termio_sweep();
  Display_close();
  printf("\n");
}

int main(int argc, char** argv, char** env)
{
  TCHAR *sreg = NULL;
  TCHAR *line = NULL;
  TCHAR *tokn = NULL;
  TCHAR promt[512]; // = "fab@smk:~/> ";

  // Read settings
  atexit(leave);
  Display_init();
  Termio_write("\e[96mSmoke Shell\e[0m Greetings...\n", 34);
  Display_redraw();
  // Read next line
  for (;;) {
    if (Environ_refresh() != 0) {
      // TODO Mark error!
      return -1;
    }
    snprintf(promt, 512, "%s@host:%s> ", usr, pwd);

    // Wait for next line
    line = Display_promt(promt);
    if (line == NULL) {
      break;
    } else if (*line == '\0') {
      continue;
    }

    // Parse the new line
    History_push(line);
    sreg = NULL;
    for (;;) {
      tokn = Parse_tokenize(line, &sreg);
      if (tokn == NULL) {
        break;
      }

      Parse_token(tokn);
      free(tokn);
    }

    Parse_flush();
  }

  return 0;
}

