#include "myframework.h"
#include <stdio.h>

void task_function(void *arg) {
    int id = *(int *)arg;
    printf("Task %d executed\n", id);
}

int main() {
    myframework_init();

    int subtasks = 5;
    int ids[5] = {1, 2, 3, 4, 5};

    // Fork tasks
    fw_handle_t *handle = fw_fork(task_function, ids, subtasks);

    // Wait for completion
    fw_join(handle);

    printf("All tasks completed!\n");
    myframework_shutdown();
    return 0;
}