#ifndef FJPOOL_H
#define FJPOOL_H

#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_WORKER 10
#define SIG_DISPATCH SIGUSR1

/* Task ID seed */
extern int taskid_seed;

/* Task queue size */
extern int fjtask_sz;

/* Worker PIDs */
extern pid_t wrkid_pid[MAX_WORKER];

/* Worker busy status */
extern int wrkid_busy[MAX_WORKER];

/* Current worker index for FIFO */
extern int wrkid_cur;

/* Task structure */
struct fjtask_t {
    void (*func)(void *arg);      /* Task function */
    void *arg;                    /* Task arguments */
    unsigned int taskid;          /* Unique task ID */
    int status;                   /* 0: pending, 1: running, 2: completed */
    int result;                   /* Task result (stored in shared memory) */
    struct fjtask_t *tnext;       /* Next task in queue */
};

/* Global task queue */
extern struct fjtask_t *fjtask;

/* Worker structure */
struct fjworker_t {
    pid_t pid;                    /* Process ID */
    unsigned int wrkid;           /* Worker ID */
    unsigned int taskid;          /* Assigned task ID (-1 if none) */
    int busy;                     /* 1 if busy, 0 if free */
    void (*func)(void *arg);      /* Current task function */
    void *arg;                    /* Current task arguments */
};

/* Worker array */
extern struct fjworker_t worker[MAX_WORKER];

/* API Prototypes */

/* Initialize the Fork-Join pool */
int fjpool_init(void);

/* Create a new task */
int fjtask_init(unsigned int *taskid, void *func, void *arg);

/* Assign a task to a worker */
int fjtask_assign_worker(unsigned int taskid, unsigned int wrkid);

/* Get a free worker (FIFO scheduler) */
int fjwrk_get_worker(void);

/* Dispatch a worker to execute its task */
int fjwrk_dispatch_worker(unsigned int wrkid);

/* Join all tasks and collect results */
int fjpool_join(void);

/* Clean up the pool */
int fjpool_destroy(void);

void fjwrk_worker(void *arg);

struct fjtask_t *fjtask_get_byid(unsigned int taskid);

#endif