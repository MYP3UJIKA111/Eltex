#include "prodcons_posix.h"

static volatile sig_atomic_t g_stop = 0;

static void consumer_sighandler(int sig) {
    (void)sig;
    g_stop = 1;
}

int run_consumer(void) {
    pid_t my_pid = getpid();

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        fprintf(stderr, "[Consumer %d] Shared memory not found. Start producer first.\n", my_pid);
        return -1;
    }

    char *shm = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);

    if (shm == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    sem_t *sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        fprintf(stderr, "[Consumer %d] Semaphore not found.\n", my_pid);
        munmap(shm, SHM_SIZE);
        return -1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = consumer_sighandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (sem_lock(sem) == 0) {
        header_t *hdr = (header_t *)shm;
        hdr->active_consumers++;
        sem_unlock(sem);
    }

    printf("[Consumer %d] Started, attached to shm and sem.\n", my_pid);

    int processed_count = 0;

    while (!g_stop) {
        if (sem_lock(sem) != 0) {
            printf("[Consumer %d] Semaphore error. Exiting.\n", my_pid);
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

            sem_unlock(sem);

            processed_count++;
            printf("[Consumer %d] Block@%d: count=%d, min=%d, max=%d\n",
                   my_pid, found_offset, count, min_val, max_val);

            usleep(400000);
            continue;
        }

        if (hdr->is_done) {
            sem_unlock(sem);
            printf("[Consumer %d] Producer is done and all blocks processed.\n", my_pid);
            break;
        }

        sem_unlock(sem);
        usleep(500000);
    }

    if (sem_lock(sem) == 0) {
        header_t *hdr = (header_t *)shm;
        if (hdr->active_consumers > 0) {
            hdr->active_consumers--;
        }
        sem_unlock(sem);
    }

    munmap(shm, SHM_SIZE);
    sem_close(sem);

    printf("[Consumer %d] Done. Processed %d blocks.\n", my_pid, processed_count);
    return 0;
}