#include "prodcons.h"

static int g_shmid = -1;
static int g_semid = -1;
static char *g_shm = NULL;
static volatile sig_atomic_t g_stop = 0;

static void producer_sighandler(int sig) {
    (void)sig;
    g_stop = 1;
}

static int align8(int size) {
    return (size + 7) & ~7;
}

int run_producer(int max_blocks) {
    key_t shm_key = get_shm_key();
    key_t sem_key = get_sem_key();

    g_shmid = shmget(shm_key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if (g_shmid == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "[Producer] Shared memory already exists.\n");
            return -1;
        }
        perror("shmget");
        return -1;
    }

    g_shm = shmat(g_shmid, NULL, 0);
    if (g_shm == (char *)-1) {
        perror("shmat");
        shmctl(g_shmid, IPC_RMID, NULL);
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

    g_semid = semget(sem_key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (g_semid == -1) {
        perror("semget");
        shmdt(g_shm);
        shmctl(g_shmid, IPC_RMID, NULL);
        return -1;
    }

    union semun arg;
    arg.val = 1;
    if (semctl(g_semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL");
        shmdt(g_shm);
        shmctl(g_shmid, IPC_RMID, NULL);
        semctl(g_semid, 0, IPC_RMID);
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

        if (sem_lock(g_semid) != 0) break;

        if (hdr->next_free + block_size > SHM_SIZE) {
            sem_unlock(g_semid);
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

        sem_unlock(g_semid);

        generated++;
        printf("[Producer] Block #%d at offset %d, count=%d\n",
               generated, new_offset, count);

        usleep(150000);
    }

    if (sem_lock(g_semid) == 0) {
        hdr->is_done = 1;
        sem_unlock(g_semid);
    }

    printf("[Producer] Generation phase done (generated=%d).\n", generated);
    printf("[Producer] Waiting for consumers to finish...\n");

    while (!g_stop) {
        if (sem_lock(g_semid) != 0) break;

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

        sem_unlock(g_semid);

        if (all_processed && consumers_done) break;

        sleep(1);
    }

    printf("[Producer] All blocks processed. Cleaning up...\n");
    printf("[Producer] Stats: generated=%d, processed=%d\n",
           hdr->total_generated, hdr->total_processed);

    shmdt(g_shm);
    shmctl(g_shmid, IPC_RMID, NULL);
    semctl(g_semid, 0, IPC_RMID);

    printf("[Producer] Done.\n");
    return 0;
}