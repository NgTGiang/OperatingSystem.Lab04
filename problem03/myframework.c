#include "myframework.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

static fw_task_t *task_queue = NULL;
static fw_worker_t workers[MAX_WORKERS];
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

void myframework_init() {
    for (int i = 0; i < MAX_WORKERS; i++) {
        workers[i].is_busy = 0;
        workers[i].current_task = NULL;
        pthread_create(&workers[i].thread_id, NULL, fw_worker_loop, (void *)(long)i);
    }
}

fw_handle_t *myframework_fork(void (*func)(void *), void *arg, int subtasks) {
    fw_handle_t *handle = malloc(sizeof(fw_handle_t));
    atomic_init(&handle->remaining_tasks, subtasks);

    pthread_mutex_lock(&queue_mutex);
    for (int i = 0; i < subtasks; i++) {
        fw_task_t *task = malloc(sizeof(fw_task_t));
        task->func = func;
        task->arg = arg;
        task->task_id = i;
        task->completion_counter = &handle->remaining_tasks;
        task->next = task_queue;
        task_queue = task;
    }
    pthread_mutex_unlock(&queue_mutex);

    // Wake up workers
    for (int i = 0; i < MAX_WORKERS; i++) {
        if (!workers[i].is_busy) {
            pthread_kill(workers[i].thread_id, SIG_DISPATCH);
        }
    }
    return handle;
}

void myframework_join(fw_handle_t *handle) {
    while (atomic_load(&handle->remaining_tasks) > 0) {
        usleep(1000); // Spin-wait (could use condition variables)
    }
    free(handle);
}

void myframework_shutdown() {
    for (int i = 0; i < MAX_WORKERS; i++) {
        pthread_cancel(workers[i].thread_id);
    }
}