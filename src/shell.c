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
#include <unistd.h>

int in_error = 0;
int fd = 4;
struct shell_cmd *__cmd = NULL;


struct shell_cmd *Shell_cmd() {
  if (__cmd == NULL) {
    __cmd = (struct shell_cmd*)malloc(sizeof(struct shell_cmd));
    memset(__cmd, 0, sizeof(struct shell_cmd));
  }
  return __cmd;
}

static void Shell_stash_show(struct shell_cmd * cmd, char* msg)
{
  if (cmd == NULL)
    cmd = Shell_cmd();
  printf("   %-20s '%s'(%d) <%d, %d, %d> {%2o}\n",
      msg, cmd->name_, cmd->argc_,
      cmd->in_, cmd->out_, cmd->err_,
      cmd->flags_);
}

int Shell_fifo () {
  return ++fd;
}

void Shell_free_cmd(struct shell_cmd* cmd) {
  struct shell_cmd* prev = cmd->prev_;
  if (cmd->name_) {
    free(cmd->name_);
  }
  if (cmd->in_name_) {
    free(cmd->in_name_);
  }
  if (cmd->out_name_) {
    free(cmd->out_name_);
  }
  if (cmd->err_name_) {
    free(cmd->err_name_);
  }

  memset(cmd, 0, sizeof(struct shell_cmd));
  cmd->prev_ = prev;
}

// int stash_depth = 0;
void Shell_stash_push()
{
  struct shell_cmd *prev = __cmd;
  __cmd = NULL;
  Shell_cmd()->prev_ = prev;

  // stash_depth++;
  Shell_stash_show(prev, " stash in");
}

void Shell_stash_pop()
{
  struct shell_cmd *prev = __cmd;
  __cmd = __cmd->prev_;
  if (prev) {
    Shell_free_cmd(prev);
    free(prev);
  }

  // stash_depth--;
  if (__cmd)
    Shell_stash_show(__cmd, " stash out");
}

void Shell_sweep() {
  in_error = 0;
  while (__cmd) {
    Shell_stash_pop();
  }
}

void Shell_exec (struct shell_cmd *cmd)
{
  char buffer[64];
  if (strcmp(cmd->name_, "exit") == 0) {
    exit(0);
  }

  snprintf(buffer, 64, "   %-20s '%s'(%d) <%d, %d, %d> {%2o}\n",
      "Execute", cmd->name_, cmd->argc_,
      cmd->in_, cmd->out_, cmd->err_,
      cmd->flags_);
  // Termio_write(buffer, strlen(buffer));
  // Display_redraw();

  if (strcmp(cmd->name_, "cd") == 0) {
    if (cmd->argc_ == 0) {
        chdir("~/");
    } else {
        chdir(cmd->argv_[0]);
    }
    return;
  }

  snprintf(buffer, 64, "/bin/%s", cmd->name_);
  int rg = access(buffer, X_OK);
  if (rg != 0) {
    switch(errno) {
      case EACCES:
        Termio_write("You don't have excecution right.\n", 33);
        break;
      case ENOENT:
      case ELOOP:
      case ENAMETOOLONG:
      case ENOTDIR:
        Termio_write("This program can't be found on the current PATH.\n", 49);
        break;
      case ETXTBSY:
        Termio_write("This program is locked by another process.\n", 33);
        break;
      default:
        Termio_write("Unable to start the program.\n", 29);
        break;
    }
    return;
  }

  switch (cmd->argc_) {
    case 0:
      snprintf(buffer, 64, "/bin/%s 2>&1", cmd->name_);
      break;
    case 1:
      snprintf(buffer, 64, "/bin/%s \"%s\" 2>&1", cmd->name_,
        cmd->argv_[0]);
      break;
    default:
    case 2:
      snprintf(buffer, 64, "/bin/%s \"%s\" \"%s\" 2>&1", cmd->name_,
        cmd->argv_[0], cmd->argv_[1]);
      break;
    case 3:
      snprintf(buffer, 64, "/bin/%s \"%s\" \"%s\" \"%s\" 2>&1", cmd->name_,
        cmd->argv_[0], cmd->argv_[1], cmd->argv_[2]);
      break;
  }
  FILE *fp = popen(buffer, "r");
  while (!feof(fp)) {
    size_t lg = fread(buffer, 1, 64, fp);
    if (lg > 0) {
      Termio_write(buffer, lg);
      Display_redraw();
    }
  }
  pclose(fp);
}

int Shell_run()
{
  if (__cmd == NULL) {
    return 0;
  }
  int fd_out;
  if (__cmd->out_ == 0) {
    __cmd->out_ = Shell_fifo();
  }
  Shell_stash_show(__cmd, "Execute");
  if (!in_error) {
    Shell_exec(__cmd);
  }
  fd_out = __cmd->out_;
  Shell_free_cmd(__cmd);
  return fd_out;
}

void Shell_error(char *error, TCHAR *token)
{
  printf("  ERR %s: '%s'\n", error, token);
  if (in_error) {
    return;
  }
  in_error = !0;
  Shell_cmd()->flags_ |= SH_ERROR;
  Termio_write(error, strlen(error));
  Termio_write("\n", 1);
  Display_redraw();
}

void Shell_stack_arg(TCHAR *token)
{
  if (Shell_cmd()->name_ == NULL) {
    Shell_cmd()->name_ = _tcsdup(token);
    // printf("  == '%s'\n", token);
  } else if (Shell_cmd()->argc_ < 64) {
    Shell_cmd()->argv_[Shell_cmd()->argc_] = _tcsdup(token);
    Shell_cmd()->argc_++;
    // printf("    -- '%s'\n", token);
  }
}

