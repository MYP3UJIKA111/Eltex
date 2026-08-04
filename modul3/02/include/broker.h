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

#define MAX_MSG_SIZE 256
#define MAX_TOPIC_SIZE 32
#define MAX_PID_SIZE 16
#define MAX_SUBSCRIBERS 100
#define MAX_PUBLISHERS 100
#define MSG_TYPE_BROKER 1
#define MSG_TYPE_SUBSCRIBE 1
#define MSG_TYPE_UNSUBSCRIBE 1

/* Структура сообщения для System V */
typedef struct {
    long mtype;
    char mtext[MAX_MSG_SIZE];
} message_t;

/* Структура подписчика */
typedef struct {
    pid_t pid;
    char topics[5][MAX_TOPIC_SIZE];
    int topic_count;
} subscriber_t;

/* Структура издателя */
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
    bool running;
    bool is_broker;
} broker_context_t;

/* Прототипы функций */
/* broker.c */
int broker_run(broker_context_t *ctx);
int broker_process_message(broker_context_t *ctx, message_t *msg);
int broker_add_subscriber(broker_context_t *ctx, pid_t pid, const char *topic);
int broker_remove_subscriber(broker_context_t *ctx, pid_t pid, const char *topic);
int broker_add_publisher(broker_context_t *ctx, pid_t pid);
void broker_remove_publisher(broker_context_t *ctx, pid_t pid);
void broker_forward_message(broker_context_t *ctx, const char *topic, const char *payload, pid_t sender_pid);

/* queue.c */
int queue_create(key_t key, int *msqid);
int queue_open(key_t key, int *msqid);
int queue_send(int msqid, long mtype, const char *text);
int queue_receive(int msqid, message_t *msg, long mtype, int flags);
int queue_destroy(int msqid);
key_t get_queue_key(void);

/* publisher.c */
int publisher_run(key_t key, const char *topic);
int publisher_send_message(int msqid, pid_t pid, const char *topic, const char *payload);

/* subscriber.c */
int subscriber_run(key_t key, char *topics[]);
int subscriber_subscribe(int msqid, pid_t pid, const char *topic);
int subscriber_unsubscribe(int msqid, pid_t pid, const char *topic);

/* signal_handler.c */
void setup_signal_handlers(void);
void signal_handler(int sig);

#endif /* BROKER_H */