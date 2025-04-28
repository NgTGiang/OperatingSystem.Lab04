#include "myframework.h"
#include <stdio.h>
#include <stdlib.h>

fw_task_t *fwtask_get_byid(unsigned int taskid) {
    fw_task_t *task = task_queue;
    while (task != NULL) {
        if (task->task_id == taskid) {
            return task;
        }
        task = task->next;
    }
    return NULL;
}

int fwtask_init(unsigned int *taskid, void *func, void *arg) {
    fw_task_t *new_task = malloc(sizeof(fw_task_t));
    if (new_task == NULL) {
        fprintf(stderr, "Task allocation failed\n");
        return -1;
    }

    static unsigned int next_taskid = 0;
    *taskid = next_taskid++;
    new_task->func = func;
    new_task->arg = arg;
    new_task->task_id = *taskid;
    new_task->completion_counter = NULL; // Set by fw_fork
    new_task->next = NULL;

    pthread_mutex_lock(&queue_mutex);
    new_task->next = task_queue;
    task_queue = new_task;
    pthread_mutex_unlock(&queue_mutex);

    return 0;
}