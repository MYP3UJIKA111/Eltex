#ifndef PRODCONS_POSIX_H
#define PRODCONS_POSIX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>

#define SHM_NAME "/prodcons_shm"
#define SEM_NAME "/prodcons_sem"
#define SHM_SIZE 16384

typedef struct {
    int head;
    int tail;
    int next_free;
    int is_done;
    int total_generated;
    int total_processed;
    int active_consumers;
} header_t;

typedef struct {
    int next;
    int count;
} node_t;

int sem_lock(sem_t *sem);
int sem_unlock(sem_t *sem);
int run_producer(int max_blocks);
int run_consumer(void);

#endif