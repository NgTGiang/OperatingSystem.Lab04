/*
  Tran Thanh Vu - MSSV: 2420012
*/
#include "bktpool.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define _GNU_SOURCE
#include <linux/sched.h>
#include <sys/syscall.h> /* Definition of SYS_* constants */

#define INFO
//#define DEBUG

/* Signal handler for SIGUSR1 in fork-based workers */
static void handle_sigusr1(int sig, siginfo_t *info, void *context)
{
    unsigned int wrkid = *(unsigned int *)info->si_value.sival_ptr;
    struct bkworker_t *wrk = &worker[wrkid];

    #ifdef INFO
    fprintf(stderr, "worker wake %d up\n", wrkid);
    #endif

    /* Execute task if assigned */
    if (wrk->func != NULL) {
        wrk->func(wrk->arg);
    }

    /* Clear task and mark as not busy */
    wrkid_busy[wrkid] = 0;
    wrk->func = NULL;
    wrk->arg = NULL;
    wrk->bktaskid = -1;
}

/* Worker function for both thread and process-based workers */
void *bkwrk_worker(void *arg)
{
    unsigned int i = *(unsigned int *)arg; /* Worker ID */
    struct bkworker_t *wrk = &worker[i];
    sigset_t set;
    struct sigaction sa;

    /* Set up signal handler for SIGUSR1 */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_sigusr1;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        fprintf(stderr, "Worker %d: Failed to set SIGUSR1 handler\n", i);
        exit(1);
    }

    /* Set up SIGQUIT to terminate worker */
    sa.sa_handler = SIG_DFL;
    sa.sa_flags = 0;
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        fprintf(stderr, "Worker %d: Failed to set SIGQUIT handler\n", i);
        exit(1);
    }

    /* Block SIGUSR1 and SIGQUIT in main loop */
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGQUIT);
    sigprocmask(SIG_BLOCK, &set, NULL);

    #ifdef DEBUG
    fprintf(stderr, "worker %u start living pid %d\n", i, getpid());
    fflush(stderr);
    #endif

    /* Main loop: wait for signals */
    while (1) {
        pause(); /* Wait for signals */
    }

    return NULL; /* Unreachable */
}

/* Create workers (thread or process-based) */
int bkwrk_create_worker()
{
    unsigned int i;

    for (i = 0; i < MAX_WORKER; i++) {
#ifdef WORK_THREAD
        void **child_stack = (void **)malloc(STACK_SIZE);
        unsigned int wrkid = i;
        pthread_t threadid;

        sigset_t set;
        int s;

        sigemptyset(&set);
        sigaddset(&set, SIGQUIT);
        sigaddset(&set, SIGUSR1);
        sigprocmask(SIG_BLOCK, &set, NULL);

        /* Stack grows down - start at top */
        void *stack_top = child_stack + STACK_SIZE;

        wrkid_tid[i] = clone(&bkwrk_worker, stack_top,
                             CLONE_VM | CLONE_FILES,
                             (void *)&wrkid);
        #ifdef INFO
        fprintf(stderr, "bkwrk_create_worker got worker %u\n", wrkid_tid[i]);
        #endif

        usleep(100);
#else
        /* Fork-based worker creation */
        pid_t pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Failed to fork worker %u\n", i);
            return -1;
        } else if (pid == 0) {
            /* Child process */
            wrkid_tid[i] = getpid();
            unsigned int wrkid = i;
            bkwrk_worker(&wrkid);
            exit(0); /* Should not reach here */
        } else {
            /* Parent process */
            wrkid_tid[i] = pid;
            #ifdef INFO
            fprintf(stderr, "bkwrk_create_worker got worker %u\n", wrkid_tid[i]);
            #endif
        }
#endif
    }

    return 0;
}

/* Assign a task to a worker */
int bktask_assign_worker(unsigned int bktaskid, unsigned int wrkid)
{
    if (wrkid >= MAX_WORKER) {
        return -1;
    }

    struct bktask_t *tsk = bktask_get_byid(bktaskid);
    if (tsk == NULL) {
        return -1;
    }

    /* Advertise I AM WORKING */
    wrkid_busy[wrkid] = 1;

    worker[wrkid].func = tsk->func;
    worker[wrkid].arg = tsk->arg;
    worker[wrkid].bktaskid = bktaskid;

    printf("Assign tsk %d wrk %d\n", tsk->bktaskid, wrkid);
    return 0;
}

/* Dispatch a worker to execute its task */
int bkwrk_dispatch_worker(unsigned int wrkid)
{
    if (wrkid >= MAX_WORKER) {
        return -1;
    }

    /* Invalid task */
    if (worker[wrkid].func == NULL) {
        return -1;
    }

#ifdef WORK_THREAD
    unsigned int tid = wrkid_tid[wrkid];

    #ifdef DEBUG
    fprintf(stderr, "bkwrk dispatch wrkid %d - send signal %u\n", wrkid, tid);
    #endif

    syscall(SYS_tkill, tid, SIG_DISPATCH);
#else
    /* Fork-based dispatch */
    pid_t pid = wrkid_tid[wrkid];
    union sigval value;
    value.sival_ptr = &wrkid; /* Pass worker ID to handler */

    #ifdef DEBUG
    fprintf(stderr, "bkwrk dispatch wrkid %d - send signal %u\n", wrkid, pid);
    #endif

    if (sigqueue(pid, SIG_DISPATCH, value) == -1) {
        fprintf(stderr, "Failed to send SIGUSR1 to worker %u\n", wrkid);
        return -1;
    }
#endif
    return 0;
}

/* Select a free worker (TODO: Implement scheduler) */
int bkwrk_get_worker()
{
    /* TODO Implement the scheduler to select the resource entity */
    return 0;
}