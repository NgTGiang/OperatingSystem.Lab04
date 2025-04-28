#include "fjpool.h"
#include <stdio.h>

struct fjtask_t *fjtask_get_byid(unsigned int taskid) {
    struct fjtask_t *task = fjtask;

    if (taskid >= (unsigned int)taskid_seed) {
        return NULL;
    }

    while (task != NULL) {
        if (task->taskid == taskid) {
            return task;
        }
        task = task->tnext;
    }

    return NULL;
}

int fjtask_init(unsigned int *taskid, void *func, void *arg) {
    struct fjtask_t *new_task = malloc(sizeof(struct fjtask_t));
    if (new_task == NULL) {
        fprintf(stderr, "Task allocation failed\n");
        return -1;
    }

    *taskid = taskid_seed++;
    fjtask_sz++;

    new_task->func = func;
    new_task->arg = arg;
    new_task->taskid = *taskid;
    new_task->status = 0; /* Pending */
    new_task->result = 0;
    new_task->tnext = fjtask;

    fjtask = new_task;

    return 0;
}

int fjtask_assign_worker(unsigned int taskid, unsigned int wrkid) {
    if (wrkid >= MAX_WORKER) {
        fprintf(stderr, "Invalid worker ID %u\n", wrkid);
        return -1;
    }

    struct fjtask_t *task = fjtask_get_byid(taskid);
    if (task == NULL) {
        fprintf(stderr, "Task %u not found\n", taskid);
        return -1;
    }

    if (wrkid_busy[wrkid]) {
        fprintf(stderr, "Worker %u is busy\n", wrkid);
        return -1;
    }

    /* Assign task to worker */
    wrkid_busy[wrkid] = 1;
    worker[wrkid].func = task->func;
    worker[wrkid].arg = task->arg;
    worker[wrkid].taskid = taskid;
    task->status = 1; /* Running */

    printf("Assigned task %u to worker %u\n", taskid, wrkid);
    return 0;
}