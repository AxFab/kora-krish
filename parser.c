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
#include "krish.h"

// Tokens: > >> < << | || & && ( ) identifer
/* Tokens
  <exec_line> := <identifier>+
  <change_out> := '>' <identifer>
               := '>>' <identifer>
               := '>' '&' <no>
               := '^' '>' <identifer>
               := '^' '>>' <identifer>
               := '^' '>' '&' <no>
  <change_in> := '<' <identifer>
              := '<' '(' <exec_run> ')'
  <exec_run> := <exec_line> <change_in>? <change_out>?
  <exec_stmt> := <exec_line> '|' <exec_stmt>
              := <exec_run> '||' <exec_stmt>
              := <exec_run> '&&' <exec_stmt>
              := '(' <exec_stmt> ')'
              := <exec_run> '&'?
*/

enum {
    /* Init a new statement, expect '(' or an identifier */
    LX_ST_STATEMENT = 0,
    /* Build up of the command, expect anything unless '(' */
    LX_ST_COMMAND,
    /* End of the command, like LX_ST_COMMAND but don't expect new args */
    LX_ST_COMMAND_IO,
    /* End of a background command, expect '||' or '&&' */
    LX_ST_COMMAND_NEXT,
    /* Redirect stdin, expect an identifier or a '(' */
    LX_ST_STDIN,
    /* Redirect stdout, expect an identifier or a '&' */
    LX_ST_STDOUT,
    /* Redirect stdout using a file descriptor */
    LX_ST_STDOUT_NO,
    /* Redirect stderr, expect '>' or '>>' */
    LX_ST_STDERR_PFX,
    /* Redirect stderr, expect an identifier or a '&' */
    LX_ST_STDERR,
    /* Redirect stderr using a file descriptor */
    LX_ST_STDERR_NO,
    /* An error happend ignore all */
    LX_ST_ERROR,
};

int lexer_state = LX_ST_STATEMENT;


static int Parse_command(char *token)
{
    int fifo;
    if (strcmp(token, "(") == 0)
        Shell_error("#1", token);
    else if (strcmp(token, "<<") == 0)
        Shell_error("#2", token);
    else if (strcmp(token, ")") == 0) {
        lexer_state = LX_ST_COMMAND;
        fifo = Shell_run();
        Shell_stash_pop();
        // Current get linked to the task we just start
    } else if (strcmp(token, "|") == 0) {
        lexer_state = LX_ST_STATEMENT;
        fifo = Shell_run();
        Shell_cmd()->in_ = fifo;
    } else if (strcmp(token, "||") == 0) {
        lexer_state = LX_ST_STATEMENT;
        fifo = Shell_run();
        // Launch task, mark failure as condition for next
    } else if (strcmp(token, "&&") == 0) {
        lexer_state = LX_ST_STATEMENT;
        fifo = Shell_run();
        // Launch task, mark success as condition for next
    } else {
        int prev_state = lexer_state;
        if (strcmp(token, "<") == 0)
            lexer_state = LX_ST_STDIN;
        else if (strcmp(token, ">") == 0) {
            lexer_state = LX_ST_STDOUT;
            Shell_cmd()->flags_ |= SH_REDIRECT_OUT | SH_TRUNCAT_OUT;
        } else if (strcmp(token, ">>") == 0) {
            lexer_state = LX_ST_STDOUT;
            Shell_cmd()->flags_ |= SH_REDIRECT_OUT | SH_TRUNCAT_OUT;
        } else if (strcmp(token, "&") == 0) {
            lexer_state = LX_ST_COMMAND_NEXT;
            Shell_cmd()->flags_ |= SH_BACKGROUND;
        } else if (strcmp(token, "^") == 0)
            lexer_state = LX_ST_STDERR_PFX;
        else
            return -1;

        if (prev_state == LX_ST_COMMAND_NEXT)
            Shell_error("#3", token);
    }

    return 0;
}


void parse_token(char *token)
{
    switch (lexer_state) {

    case LX_ST_STATEMENT:
        if (strcmp(token, "(") == 0)
            Shell_stash_push();
        else {
            Shell_stack_arg(token);
            lexer_state = LX_ST_COMMAND;
        }
        break;

    case LX_ST_COMMAND:
        if (Parse_command(token) != 0)
            Shell_stack_arg(token);
        break;

    case LX_ST_COMMAND_IO:
    case LX_ST_COMMAND_NEXT:
        if (Parse_command(token) != 0)
            Shell_error("#4", token);
        break;

    case LX_ST_STDIN:
        if (strcmp(token, "(") == 0) {
            int fd;
            fd = Shell_fifo();
            Shell_cmd()->in_ = fd;
            Shell_stash_push();
            Shell_cmd()->out_ = fd;
            lexer_state = LX_ST_COMMAND;
        } else {
            Shell_cmd()->in_name_ = strdup(token);
            lexer_state = LX_ST_COMMAND_IO;
        }
        break;

    case LX_ST_STDOUT:
        if (strcmp(token, "&") == 0)
            lexer_state = LX_ST_STDOUT_NO;
        else {
            Shell_cmd()->out_name_ = strdup(token);
            lexer_state = LX_ST_COMMAND_IO;
        }
        break;

    case LX_ST_STDOUT_NO:
        Shell_cmd()->out_ = strtol(token, NULL, 10);
        lexer_state = LX_ST_COMMAND_IO;
        break;

    case LX_ST_STDERR_PFX:
        if (strcmp(token, ">") == 0) {
            lexer_state = LX_ST_STDERR;
            Shell_cmd()->flags_ |= SH_TRUNCAT_ERR;
        } else if (strcmp(token, ">>") == 0) {
            lexer_state = LX_ST_STDERR;
            Shell_cmd()->flags_ &= ~SH_TRUNCAT_ERR;
        } else
            Shell_error("#5", token);
        break;

    case LX_ST_STDERR:
        if (strcmp(token, "&") == 0)
            lexer_state = LX_ST_STDERR_NO;
        else {
            Shell_cmd()->err_name_ = strdup(token);
            lexer_state = LX_ST_COMMAND_IO;
        }
        break;

    case LX_ST_STDERR_NO:
        Shell_cmd()->err_ = strtol(token, NULL, 10);
        lexer_state = LX_ST_COMMAND_IO;
        break;

    default:
        Shell_error("#6", token);
    case LX_ST_ERROR:
        break;
    }
}

void parse_flush()
{
    if (lexer_state == LX_ST_COMMAND ||
        lexer_state == LX_ST_COMMAND_IO ||
        lexer_state == LX_ST_COMMAND_NEXT)
        Shell_run();
    else
        Shell_error("#7", NULL);

    Shell_sweep();
    lexer_state = LX_ST_STATEMENT;
}


char *parse_tokenize(const char *line, const char **sreg)
{
    const char *OPS = "<>|&^()";
    const char *OPS2 = "<>|&";
    const char *QUOTE = "'\"`";
    int lg, c1, c2, s = 0;
    const char *tok1 = *sreg;
    char *tok2;
    if (tok1 == NULL)
        tok1 = line;

    while (isblank(*tok1) || *tok1 == '\n')
        tok1++;

    if (*tok1 == '\0')
        return NULL;

    *sreg = tok1;
    c1 = *tok1;
    if (strchr(OPS, c1) != NULL) {
        tok1++;
        c2 = *tok1;
        if (c2 == c1 && strchr(OPS2, c1) != NULL)
            tok1++;
    } else if (strchr(QUOTE, c1) != NULL) {
        c2 = *tok1;
        tok1++;
        *sreg = tok1;
        while (c2 && c1 != c2) {
            c2 = tok1[1];
            tok1++;
        }
        s = 1;
        if (c2)
            tok1++;
    } else {
        while (strchr(OPS, *tok1) == NULL && *tok1 != '\0' && *tok1 != '\n' && (*tok1 < 0 || !isblank(*tok1)))
            tok1++;
    }

    lg = tok1 - (*sreg);
    if (lg == 0) {
        tok1++;
        lg = tok1 - (*sreg);
    }

    tok2 = (char *)malloc((lg + 1) * sizeof(char));
    memcpy(tok2, *sreg, lg * sizeof(char));
    tok2[lg - s] = '\0';
    *sreg = tok1;
    return tok2;
}
