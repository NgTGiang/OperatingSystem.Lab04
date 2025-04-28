#include "fjpool.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

void fjwrk_worker(void *arg) {
    unsigned int wrkid = *(unsigned int *)arg;
    sigset_t set;
    int sig;
    int s;

    /* Set up signal mask */
    sigemptyset(&set);
    sigaddset(&set, SIG_DISPATCH);
    sigaddset(&set, SIGQUIT);

    fprintf(stderr, "Worker %u (PID %d) started\n", wrkid, getpid());

    while (1) {
        /* Wait for SIGUSR1 or SIGQUIT */
        s = sigwait(&set, &sig);
        if (s != 0) {
            continue;
        }

        if (sig == SIGQUIT) {
            fprintf(stderr, "Worker %u exiting\n", wrkid);
            exit(0);
        }

        fprintf(stderr, "Worker %u woke up\n", wrkid);

        /* Execute task */
        if (worker[wrkid].func != NULL) {
            worker[wrkid].func(worker[wrkid].arg);
            struct fjtask_t *task = fjtask_get_byid(worker[wrkid].taskid);
            if (task != NULL) {
                task->status = 2; /* Completed */
                task->result = *(int *)worker[wrkid].arg; /* Store arg as result */
            }
        }

        /* Mark worker as free */
        wrkid_busy[wrkid] = 0;
        worker[wrkid].func = NULL;
        worker[wrkid].arg = NULL;
        worker[wrkid].taskid = -1;
    }
}

int fjwrk_get_worker(void) {
    /* FIFO scheduler: check workers in order */
    for (unsigned int i = 0; i < MAX_WORKER; i++) {
        unsigned int idx = (wrkid_cur + i) % MAX_WORKER;
        if (!wrkid_busy[idx]) {
            wrkid_cur = (idx + 1) % MAX_WORKER; /* Update for next call */
            return idx;
        }
    }
    fprintf(stderr, "No free workers\n");
    return -1;
}

int fjwrk_dispatch_worker(unsigned int wrkid) {
    if (wrkid >= MAX_WORKER || wrkid_pid[wrkid] == 0) {
        fprintf(stderr, "Invalid worker %u\n", wrkid);
        return -1;
    }

    if (worker[wrkid].func == NULL) {
        fprintf(stderr, "No task assigned to worker %u\n", wrkid);
        return -1;
    }

    fprintf(stderr, "Dispatching worker %u (PID %d)\n", wrkid, wrkid_pid[wrkid]);
    kill(wrkid_pid[wrkid], SIG_DISPATCH);

    return 0;
}