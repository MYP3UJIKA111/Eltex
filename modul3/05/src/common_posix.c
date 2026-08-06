#include "prodcons_posix.h"

int sem_lock(sem_t *sem) {
    while (sem_wait(sem) == -1) {
        if (errno == EINTR) continue;
        if (errno == EINVAL) {
            perror("sem_wait: invalid semaphore");
            return -1;
        }
        perror("sem_wait");
        return -1;
    }
    return 0;
}

int sem_unlock(sem_t *sem) {
    if (sem_post(sem) == -1) {
        if (errno == EINVAL) {
            return -1;
        }
        perror("sem_post");
        return -1;
    }
    return 0;
}