#include "fjpool.h"
#include <stdio.h>
#include <unistd.h>

/* Global variables */
int taskid_seed = 0;
int fjtask_sz = 0;
pid_t wrkid_pid[MAX_WORKER] = {0};
int wrkid_busy[MAX_WORKER] = {0};
int wrkid_cur = 0;
struct fjtask_t *fjtask = NULL;
struct fjworker_t worker[MAX_WORKER] = {0};

int fjpool_init(void) {
    unsigned int i;
    sigset_t set;

    /* Block SIGUSR1 and SIGQUIT in parent */
    sigemptyset(&set);
    sigaddset(&set, SIG_DISPATCH);
    sigaddset(&set, SIGQUIT);
    sigprocmask(SIG_BLOCK, &set, NULL);

    /* Create MAX_WORKER child processes */
    for (i = 0; i < MAX_WORKER; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            fprintf(stderr, "fork failed for worker %u\n", i);
            return -1;
        } else if (pid == 0) {
            /* Child process: run worker loop */
            fjwrk_worker(&i);
            exit(0); /* Should not reach here */
        } else {
            /* Parent: store PID and initialize worker */
            wrkid_pid[i] = pid;
            worker[i].pid = pid;
            worker[i].wrkid = i;
            worker[i].taskid = -1;
            worker[i].busy = 0;
            worker[i].func = NULL;
            worker[i].arg = NULL;
            fprintf(stderr, "Created worker %u with PID %d\n", i, pid);
        }
        usleep(100); /* Avoid overwhelming system */
    }

    return 0;
}

int fjpool_join(void) {
    struct fjtask_t *task = fjtask;
    int completed = 0;

    /* Wait for all tasks to complete */
    while (task != NULL) {
        if (task->status == 2) {
            completed++;
        }
        task = task->tnext;
    }

    /* If all tasks are completed, wait for workers */
    if (completed == fjtask_sz) {
        for (unsigned int i = 0; i < MAX_WORKER; i++) {
            if (wrkid_pid[i] != 0) {
                waitpid(wrkid_pid[i], NULL, 0);
            }
        }
        return 0;
    }

    return -1; /* Not all tasks completed */
}

int fjpool_destroy(void) {
    unsigned int i;

    /* Send SIGQUIT to all workers */
    for (i = 0; i < MAX_WORKER; i++) {
        if (wrkid_pid[i] != 0) {
            kill(wrkid_pid[i], SIGQUIT);
            waitpid(wrkid_pid[i], NULL, 0);
            wrkid_pid[i] = 0;
        }
    }

    /* Free task queue */
    struct fjtask_t *task = fjtask;
    while (task != NULL) {
        struct fjtask_t *next = task->tnext;
        free(task);
        task = next;
    }
    fjtask = NULL;
    fjtask_sz = 0;
    taskid_seed = 0;
    wrkid_cur = 0;

    return 0;
}