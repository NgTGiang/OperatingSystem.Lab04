#include "bktpool.h"
#include <sys/mman.h>
#include <string.h>

/* Global variables */
int taskid_seed = 0;
int wrkid_tid[MAX_WORKER];
int *wrkid_busy;
int wrkid_cur = 0;
struct bktask_t *bktask = NULL;
int bktask_sz = 0;
struct bkworker_t *worker;

/* Initialize task pool and shared memory */
int bktpool_init()
{
    /* Initialize shared memory for wrkid_busy */
    wrkid_busy = mmap(NULL, MAX_WORKER * sizeof(int), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (wrkid_busy == MAP_FAILED)
        return -1;
    memset(wrkid_busy, 0, MAX_WORKER * sizeof(int));

    /* Initialize shared memory for worker */
    worker = mmap(NULL, MAX_WORKER * sizeof(struct bkworker_t), PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (worker == MAP_FAILED)
        return -1;
    memset(worker, 0, MAX_WORKER * sizeof(struct bkworker_t));
    for (unsigned int i = 0; i < MAX_WORKER; i++) {
        worker[i].wrkid = i;
        worker[i].bktaskid = -1;
    }

    return bkwrk_create_worker();
}