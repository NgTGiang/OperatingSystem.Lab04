#ifndef MYFRAMEWORK_H
#define MYFRAMEWORK_H

#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>

#define MAX_WORKERS 10
#define SIG_DISPATCH SIGUSR1
#define SIG_COMPLETE SIGUSR2

typedef struct fw_task {
    void (*func)(void *);
    void *arg;
    int task_id;
    atomic_int *completion_counter; // For join synchronization
    struct fw_task *next;
} fw_task_t;

typedef struct {
    pthread_t thread_id;
    int is_busy;
    fw_task_t *current_task;
} fw_worker_t;

typedef struct {
    atomic_int remaining_tasks; // Tracks pending tasks
} fw_handle_t;

// API Functions
void myframework_init();
fw_handle_t *myframework_fork(void (*func)(void *), void *arg, int subtasks);
void myframework_join(fw_handle_t *handle);
void myframework_shutdown();

#endif