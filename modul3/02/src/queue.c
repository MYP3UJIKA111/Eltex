#include "broker.h"

key_t get_queue_key(void) {
    const char *path = "/tmp";
    int proj_id = 'B';

    key_t key = ftok(path, proj_id);
    if (key == (key_t)-1) {
        perror("ftok");
        key = 0x50554253; /* Резервный ключ PUBS */
    }
    return key;
}

int queue_create(key_t key, int *msqid) {
    int id = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if (id == -1) {
        return -1;
    }
    *msqid = id;
    return 0;
}

int queue_open(key_t key, int *msqid) {
    int id = msgget(key, 0);
    if (id == -1) {
        return -1;
    }
    *msqid = id;
    return 0;
}

int queue_send(int msqid, long mtype, const char *text) {
    message_t msg;

    msg.mtype = mtype;
    strncpy(msg.mtext, text, MAX_MSG_SIZE - 1);
    msg.mtext[MAX_MSG_SIZE - 1] = '\0';

    size_t len = strlen(msg.mtext) + 1;

    /* Пытаемся отправить, обрабатывая переполнение очереди */
    for (int i = 0; i < 20; ++i) {
        if (msgsnd(msqid, &msg, len, IPC_NOWAIT) == 0) {
            return 0;
        }
        if (errno == EAGAIN) {
            usleep(50000);
            continue;
        }
        if (errno == EINTR) {
            continue;
        }
        return -1;
    }
    errno = EAGAIN;
    return -1;
}

int queue_destroy(int msqid) {
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        if (errno != EINVAL) { /* EINVAL = уже удалена */
            perror("msgctl IPC_RMID");
            return -1;
        }
    }
    return 0;
}