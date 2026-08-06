#include "prodcons.h"

static volatile sig_atomic_t g_stop = 0;

static void consumer_sighandler(int sig) {
    (void)sig;
    g_stop = 1;
}

int run_consumer(void) {
    key_t shm_key = get_shm_key();
    key_t sem_key = get_sem_key();
    pid_t my_pid = getpid();

    int shmid = shmget(shm_key, 0, 0);
    if (shmid == -1) {
        fprintf(stderr, "[Consumer %d] Shared memory not found.\n", my_pid);
        return -1;
    }

    char *shm = shmat(shmid, NULL, 0);
    if (shm == (char *)-1) {
        perror("shmat");
        return -1;
    }

    int semid = semget(sem_key, 0, 0);
    if (semid == -1) {
        fprintf(stderr, "[Consumer %d] Semaphore not found.\n", my_pid);
        shmdt(shm);
        return -1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = consumer_sighandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (sem_lock(semid) == 0) {
        header_t *hdr = (header_t *)shm;
        hdr->active_consumers++;
        sem_unlock(semid);
    }

    printf("[Consumer %d] Started, attached to shm and sem.\n", my_pid);

    int processed_count = 0;

    while (!g_stop) {
        if (sem_lock(semid) != 0) {
            printf("[Consumer %d] Semaphore removed. Exiting.\n", my_pid);
            break;
        }

        header_t *hdr = (header_t *)shm;

        int found_offset = 0;
        int off = hdr->head;
        while (off != 0) {
            node_t *node = (node_t *)(shm + off);
            if (node->count != 0) {
                found_offset = off;
                break;
            }
            off = node->next;
        }

        if (found_offset != 0) {
            node_t *node = (node_t *)(shm + found_offset);
            int count = node->count;
            int *data = (int *)(node + 1);

            int min_val = data[0];
            int max_val = data[0];
            for (int i = 1; i < count; i++) {
                if (data[i] < min_val) min_val = data[i];
                if (data[i] > max_val) max_val = data[i];
            }

            node->count = 0;
            hdr->total_processed++;

            sem_unlock(semid);

            processed_count++;
            printf("[Consumer %d] Block@%d: count=%d, min=%d, max=%d\n",
                   my_pid, found_offset, count, min_val, max_val);

            usleep(400000);
            continue;
        }

        if (hdr->is_done) {
            sem_unlock(semid);
            printf("[Consumer %d] Producer is done and all blocks processed.\n", my_pid);
            break;
        }

        sem_unlock(semid);
        usleep(500000);
    }

    if (sem_lock(semid) == 0) {
        header_t *hdr = (header_t *)shm;
        if (hdr->active_consumers > 0) {
            hdr->active_consumers--;
        }
        sem_unlock(semid);
    }

    shmdt(shm);

    printf("[Consumer %d] Done. Processed %d blocks.\n", my_pid, processed_count);
    return 0;
}