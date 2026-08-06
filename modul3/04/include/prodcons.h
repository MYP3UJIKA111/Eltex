#ifndef PRODCONS_H
#define PRODCONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_SIZE 16384
#define SHM_PROJ_ID 'M'
#define SEM_PROJ_ID 'S'
#define KEY_PATH "/tmp"

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

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

key_t get_shm_key(void);
key_t get_sem_key(void);
int sem_lock(int semid);
int sem_unlock(int semid);
int run_producer(int max_blocks);
int run_consumer(void);

#endif