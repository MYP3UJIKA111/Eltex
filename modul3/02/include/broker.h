#ifndef BROKER_H
#define BROKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_MSG_SIZE 1024
#define MAX_TOPIC_SIZE 64
#define MAX_SUBSCRIBERS 100
#define MAX_PUBLISHERS 100
#define MAX_TOPICS_PER_SUB 10

/* Приоритет для сообщений брокеру */
#define MSG_TYPE_BROKER 1L

/* Максимальное время ожидания очистки очереди (мс) */
#define SHUTDOWN_TIMEOUT_MS 3000

/* Структура сообщения System V */
typedef struct {
    long mtype;
    char mtext[MAX_MSG_SIZE];
} message_t;

/* Подписчик: pid + список тем */
typedef struct {
    pid_t pid;
    char topics[MAX_TOPICS_PER_SUB][MAX_TOPIC_SIZE];
    int topic_count;
} subscriber_t;

/* Издатель: только pid */
typedef struct {
    pid_t pid;
} publisher_t;

/* Контекст брокера */
typedef struct {
    int msqid;
    key_t key;
    subscriber_t subscribers[MAX_SUBSCRIBERS];
    int subscriber_count;
    publisher_t publishers[MAX_PUBLISHERS];
    int publisher_count;
    volatile sig_atomic_t running;
    bool is_broker;
} broker_context_t;

/* broker.c */
int broker_run(broker_context_t *ctx);

/* queue.c */
int queue_create(key_t key, int *msqid);
int queue_open(key_t key, int *msqid);
int queue_send(int msqid, long mtype, const char *text);
int queue_destroy(int msqid);
key_t get_queue_key(void);

/* publisher.c */
int publisher_run(key_t key, const char *topic);

/* subscriber.c */
int subscriber_run(key_t key, char *topics[], int topic_count);

#endif /* BROKER_H */