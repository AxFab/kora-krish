#include <stdio.h>
#include "krish.h"
#include <kora/llist.h>
#include <kora/bbtree.h>
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

static void job_run(job_t *job)
{
    int bytes;
    char buf[64];
    if (job->flags & JB_TTYOUT) {
        do {
            bytes = read(job->fstd[1], buf, 64);
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


