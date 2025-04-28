#include "fjpool.h"
#include <stdio.h>
#include <unistd.h>

void compute_task(void *arg) {
    int id = *(int *)arg;
    printf("Task %d computed with value %d\n", id, id);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    unsigned int tid[3];
    int wid[3];
    int id[3] = {1, 2, 5};
    int ret;

    /* Initialize pool */
    ret = fjpool_init();
    if (ret != 0) {
        fprintf(stderr, "Pool initialization failed\n");
        return -1;
    }

    /* Create tasks */
    for (int i = 0; i < 3; i++) {
        ret = fjtask_init(&tid[i], compute_task, &id[i]);
        if (ret != 0) {
            fprintf(stderr, "Task %d initialization failed\n", i);
            fjpool_destroy();
            return -1;
        }
    }

    /* Assign and dispatch tasks */
    for (int i = 0; i < 3; i++) {
        wid[i] = fjwrk_get_worker();
        if (wid[i] == -1) {
            fprintf(stderr, "No worker available for task %u\n", tid[i]);
            fjpool_destroy();
            return -1;
        }
        ret = fjtask_assign_worker(tid[i], wid[i]);
        if (ret != 0) {
            fprintf(stderr, "Task %u assignment failed\n", tid[i]);
            fjpool_destroy();
            return -1;
        }
        ret = fjwrk_dispatch_worker(wid[i]);
        if (ret != 0) {
            fprintf(stderr, "Task %u dispatch failed\n", tid[i]);
            fjpool_destroy();
            return -1;
        }
    }

    /* Join tasks */
    ret = fjpool_join();
    if (ret != 0) {
        fprintf(stderr, "Join failed\n");
        fjpool_destroy();
        return -1;
    }

    /* Print results */
    struct fjtask_t *task = fjtask;
    while (task != NULL) {
        printf("Task %u result: %d\n", task->taskid, task->result);
        task = task->tnext;
    }

    /* Clean up */
    ret = fjpool_destroy();
    if (ret != 0) {
        fprintf(stderr, "Pool destruction failed\n");
        return -1;
    }

    return 0;
}