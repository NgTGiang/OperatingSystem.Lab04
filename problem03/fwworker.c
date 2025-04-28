#include "myframework.h"
#include <signal.h>
#include <stdio.h>

void *fw_worker_loop(void *arg) {
    int worker_id = (int)(long)arg;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIG_DISPATCH);

    while (1) {
        int sig;
        if (sigwait(&mask, &sig) != 0) {
            continue;
        }

        pthread_mutex_lock(&queue_mutex);
        if (task_queue) {
            fw_task_t *task = task_queue;
            task_queue = task_queue->next;
            workers[worker_id].is_busy = 1;
            workers[worker_id].current_task = task;
            pthread_mutex_unlock(&queue_mutex);

            // Execute task
            task->func(task->arg);

            // Free task argument and task
            free(task->arg);
            atomic_fetch_sub(task->completion_counter, 1);
            free(task);

            pthread_mutex_lock(&queue_mutex);
            workers[worker_id].is_busy = 0;
            workers[worker_id].current_task = NULL;
            pthread_mutex_unlock(&queue_mutex);
        } else {
            pthread_mutex_unlock(&queue_mutex);
        }
    }
    return NULL;
}