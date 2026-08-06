#include "prodcons_posix.h"

static char *g_shm = NULL;
static sem_t *g_sem = NULL;
static volatile sig_atomic_t g_stop = 0;

static void producer_sighandler(int sig) {
    (void)sig;
    g_stop = 1;
}

static int align8(int size) {
    return (size + 7) & ~7;
}

int run_producer(int max_blocks) {
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR | O_EXCL, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return -1;
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return -1;
    }

    g_shm = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);

    if (g_shm == MAP_FAILED) {
        perror("mmap");
        shm_unlink(SHM_NAME);
        return -1;
    }

    header_t *hdr = (header_t *)g_shm;
    hdr->head = 0;
    hdr->tail = 0;
    hdr->next_free = align8((int)sizeof(header_t));
    hdr->is_done = 0;
    hdr->total_generated = 0;
    hdr->total_processed = 0;
    hdr->active_consumers = 0;

    g_sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (g_sem == SEM_FAILED) {
        perror("sem_open");
        munmap(g_shm, SHM_SIZE);
        shm_unlink(SHM_NAME);
        return -1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = producer_sighandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    srand((unsigned)time(NULL) ^ (unsigned)getpid());

    printf("[Producer] Started. pid=%d, shm_size=%d\n", getpid(), SHM_SIZE);

    int generated = 0;

    while (!g_stop && (max_blocks == 0 || generated < max_blocks)) {
        int count = 5 + rand() % 11;
        int block_size = align8((int)(sizeof(node_t) + count * sizeof(int)));

        if (sem_lock(g_sem) != 0) break;

        if (hdr->next_free + block_size > SHM_SIZE) {
            sem_unlock(g_sem);
            printf("[Producer] Shared memory full.\n");
            break;
        }

        int new_offset = hdr->next_free;
        node_t *node = (node_t *)(g_shm + new_offset);
        node->next = 0;
        node->count = count;

        int *data = (int *)(node + 1);
        for (int i = 0; i < count; i++) {
            data[i] = rand() % 10000;
        }

        if (hdr->tail != 0) {
            node_t *tail_node = (node_t *)(g_shm + hdr->tail);
            tail_node->next = new_offset;
        } else {
            hdr->head = new_offset;
        }
        hdr->tail = new_offset;
        hdr->next_free += block_size;
        hdr->total_generated++;

        sem_unlock(g_sem);

        generated++;
        printf("[Producer] Block #%d at offset %d, count=%d\n",
               generated, new_offset, count);

        usleep(150000);
    }

    if (sem_lock(g_sem) == 0) {
        hdr->is_done = 1;
        sem_unlock(g_sem);
    }

    printf("[Producer] Generation phase done (generated=%d).\n", generated);
    printf("[Producer] Waiting for consumers to finish...\n");

    while (!g_stop) {
        if (sem_lock(g_sem) != 0) break;

        int all_processed = 1;
        int off = hdr->head;
        while (off != 0) {
            node_t *node = (node_t *)(g_shm + off);
            if (node->count != 0) {
                all_processed = 0;
                break;
            }
            off = node->next;
        }

        int consumers_done = (hdr->active_consumers == 0);

        sem_unlock(g_sem);

        if (all_processed && consumers_done) break;

        sleep(1);
    }

    printf("[Producer] All blocks processed. Cleaning up...\n");
    printf("[Producer] Stats: generated=%d, processed=%d\n",
           hdr->total_generated, hdr->total_processed);

    munmap(g_shm, SHM_SIZE);
    sem_close(g_sem);
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);

    printf("[Producer] Done.\n");
    return 0;
}