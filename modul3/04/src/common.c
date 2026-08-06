#include "prodcons.h"

key_t get_shm_key(void) {
    key_t key = ftok(KEY_PATH, SHM_PROJ_ID);
    if (key == (key_t)-1) {
        perror("ftok for shm");
        key = 0x53484D53;
    }
    return key;
}

key_t get_sem_key(void) {
    key_t key = ftok(KEY_PATH, SEM_PROJ_ID);
    if (key == (key_t)-1) {
        perror("ftok for sem");
        key = 0x53454D53;
    }
    return key;
}

int sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    while (semop(semid, &op, 1) == -1) {
        if (errno == EINTR) continue;
        if (errno == EINVAL || errno == EIDRM) {
            return -1;
        }
        perror("semop lock");
        return -1;
    }
    return 0;
}

int sem_unlock(int semid) {
    struct sembuf op = {0, 1, 0};
    if (semop(semid, &op, 1) == -1) {
        if (errno == EINVAL || errno == EIDRM) {
            return -1;
        }
        perror("semop unlock");
        return -1;
    }
    return 0;
}