#include "myframework.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

fw_task_t *task_queue = NULL;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static fw_worker_t workers[MAX_WORKERS];

void myframework_init(void) {
    for (int i = 0; i < MAX_WORKERS; i++) {
        workers[i].is_busy = 0;
        workers[i].current_task = NULL;
        pthread_create(&workers[i].thread_id, NULL, fw_worker_loop, (void *)(long)i);
    }
}

fw_handle_t *fw_fork(void (*func)(void *), void *arg, int subtasks) {
    fw_handle_t *handle = malloc(sizeof(fw_handle_t));
    if (!handle) {
        fprintf(stderr, "Handle allocation failed\n");
        return NULL;
    }
    atomic_init(&handle->remaining_tasks, subtasks);

    pthread_mutex_lock(&queue_mutex);
    int *ids = (int *)arg;
    for (int i = 0; i < subtasks; i++) {
        fw_task_t *task = malloc(sizeof(fw_task_t));
        if (!task) {
            fprintf(stderr, "Task allocation failed\n");
            pthread_mutex_unlock(&queue_mutex);
            return NULL;
        }
        task->func = func;
        task->arg = malloc(sizeof(int));
        if (!task->arg) {
            fprintf(stderr, "Task arg allocation failed\n");
            free(task);
            pthread_mutex_unlock(&queue_mutex);
            return NULL;
        }
        *(int *)task->arg = ids[i]; // Copy individual ID
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

void fw_join(fw_handle_t *handle) {
    while (atomic_load(&handle->remaining_tasks) > 0) {
        usleep(1000); // Spin-wait (could use condition variables)
    }
    free(handle);
}

void myframework_shutdown(void) {
    for (int i = 0; i < MAX_WORKERS; i++) {
        pthread_cancel(workers[i].thread_id);
        pthread_join(workers[i].thread_id, NULL);
    }
}