#include "myframework.h"
#include <signal.h>
#include <stdio.h>

void *fw_worker_loop(void *arg) {
    int worker_id = (int)(long)arg;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIG_DISPATCH);
    sigaddset(&mask, SIG_COMPLETE);

    while (1) {
        int sig;
        sigwait(&mask, &sig);

        pthread_mutex_lock(&queue_mutex);
        if (task_queue) {
            fw_task_t *task = task_queue;
            task_queue = task_queue->next;
            workers[worker_id].is_busy = 1;
            workers[worker_id].current_task = task;
            pthread_mutex_unlock(&queue_mutex);

            // Execute task
            task->func(task->arg);

            // Notify completion
            atomic_fetch_sub(task->completion_counter, 1);
            free(task);
            workers[worker_id].is_busy = 0;
        } else {
            pthread_mutex_unlock(&queue_mutex);
        }
    }
    return NULL;
}