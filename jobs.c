/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2021  <Fabien Bavent>
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
#include <stdio.h>
#include "krish.h"
#include "llist.h"
#include "bbtree.h"
#include <threads.h>
#include <unistd.h>

struct job {
    int pid;
    int flags;
    char *name;
    char **argv;
    int argc;
    int fstd[3];
    char *nstdin;
    char *nstdout;
    char *nstderr;
    llnode_t node;
    bbnode_t tnode;
    termio_t *tty;
    thrd_t thrd;
};

mtx_t shell_mtx;
bbtree_t shell_jobs;
llhead_t shell_stack;

#define JB_FGSTACK  1
#define JB_TTYOUT  2


static void job_print(job_t *job)
{

}

void job_init()
{
    mtx_init(&shell_mtx, mtx_plain);
    llist_init(&shell_stack);
    bbtree_init(&shell_jobs);
}

void job_dump()
{
    job_t *job;
    mtx_lock(&shell_mtx);
    for ll_each(&shell_stack, job, job_t, node)
        job_print(job);

    job = bbtree_first(&shell_jobs, job_t, tnode);
    for (; job; job = bbtree_next(&job->tnode, job_t, tnode)) {
        if (job->flags & JB_FGSTACK)
            continue;
        job_print(job);
    }
    mtx_unlock(&shell_mtx);
}

job_t *job_by_pid(int pid)
{
    mtx_lock(&shell_mtx);
    job_t *job = bbtree_search_eq(&shell_jobs, pid, job_t, tnode);
    mtx_unlock(&shell_mtx);
    return job;
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

job_t *job_create(termio_t *tty, const char *name)
{
    job_t *job = calloc(sizeof(job_t), 1);
    job->tty = tty;
    job->name = strdup(name);
    job->flags = JB_FGSTACK | JB_TTYOUT;
    job->fstd[0] = -1;
    job->fstd[1] = -1;
    job->fstd[2] = -1;
    return job;
}

void job_destroy(job_t *job)
{
    if (job->fstd[0] >= 0)
        close(job->fstd[0]);
    if (job->fstd[1] >= 0)
        close(job->fstd[1]);
    if (job->fstd[1] != job->fstd[2])
        close(job->fstd[2]);
    free(job->name);
    free(job);
}

void job_args(job_t *job, int argc, char **argv)
{
    job->argc = argc;
    job->argv = argv;
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#define PIPE_BUF_SIZE  512

void job_run(job_t *job)
{
    int bytes;
    char buf[PIPE_BUF_SIZE];
    if (job->flags & JB_TTYOUT) {
        do {
            bytes = read(job->fstd[1], buf, PIPE_BUF_SIZE);
            // TODO -- What about multi-bytes characters or escape sequences
            terminal_write(job->tty, buf, bytes);
        } while (bytes > 0);
    } else {
        // TODO - waitpid(job->pid);
    }

    terminal_pop_job(job->tty, job);
    printf("Job terminated %d\n", job->pid);
    if (job->flags & JB_FGSTACK)
        shell_prompt(job->tty);

    mtx_lock(&shell_mtx);
    bbtree_remove(&shell_jobs, job->pid);
    mtx_unlock(&shell_mtx);
    job_destroy(job);
    thrd_detach(thrd_current());
    thrd_exit(0);
}

void job_start(job_t *job)
{
    char buf[64];
    printf(" - %s [%d, %d, %d]\n", job->name, job->fstd[0], job->fstd[1], job->fstd[2]);
    int pid = __exec(job->name, job->argv, NULL, job->fstd);
    if (pid < 0) {
        // TODO Know why?
        sprintf(buf, "- '%s': Unable to start command\n", job->name);
        terminal_puts(job->tty, buf);

        mtx_lock(&shell_mtx);
        bbtree_remove(&shell_jobs, job->pid);
        mtx_unlock(&shell_mtx);
        job_destroy(job);
        return;
    }

    job->pid = pid;
    job->tnode.value_ = pid;
    mtx_lock(&shell_mtx);
    bbtree_insert(&shell_jobs, &job->tnode);
    mtx_unlock(&shell_mtx);
    thrd_create(&job->thrd, (thrd_start_t)job_run, job);
    terminal_push_job(job->tty, job);
    printf("Job started %d\n", pid);
}

void job_close_stdin(job_t *job)
{
    if (job->fstd[0] >= 0) {
        close(job->fstd[0]);
        job->fstd[0] = -1;
    }
}

void job_puts(job_t *job, const char *str)
{
    int lg = strlen(str);
    if (lg > 0)
        write(job->fstd[0], str, lg);
    write(job->fstd[0], "\n", 1);
}

job_t *job_background(job_t *job)
{
    job->flags &= ~JB_FGSTACK;
    return NULL;
}
