#include "broker.h"

key_t get_queue_key(void) {
    /* Используем ftok для генерации ключа */
    const char *path = "/tmp";
    int proj_id = 'B';
    
    key_t key = ftok(path, proj_id);
    if (key == -1) {
        perror("ftok");
        /* Если ftok не работает, используем фиксированный ключ */
        key = 0x1234;
    }
    return key;
}

int queue_create(key_t key, int *msqid) {
    int id;
    
    /* Создаем очередь с правами 0666 */
    id = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if (id == -1) {
        return -1;
    }
    
    *msqid = id;
    return 0;
}

int queue_open(key_t key, int *msqid) {
    int id;
    
    id = msgget(key, 0666);
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
    
    if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, IPC_NOWAIT) == -1) {
        perror("msgsnd");
        return -1;
    }
    
    return 0;
}

int queue_receive(int msqid, message_t *msg, long mtype, int flags) {
    ssize_t bytes;
    
    bytes = msgrcv(msqid, msg, MAX_MSG_SIZE, mtype, flags);
    if (bytes == -1) {
        return -1;
    }
    
    msg->mtext[bytes] = '\0';
    return 0;
}

int queue_destroy(int msqid) {
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl IPC_RMID");
        return -1;
    }
    return 0;
}