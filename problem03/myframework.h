#ifndef MYFRAMEWORK_H
#define MYFRAMEWORK_H

#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdlib.h>

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

// Global task queue and mutex
extern fw_task_t *task_queue;
extern pthread_mutex_t queue_mutex;

// API Functions
void myframework_init(void);
fw_handle_t *fw_fork(void (*func)(void *), void *arg, int subtasks);
void fw_join(fw_handle_t *handle);
void myframework_shutdown(void);

// Helper function for task lookup
fw_task_t *fwtask_get_byid(unsigned int taskid);

void *fw_worker_loop(void *arg);

#endif