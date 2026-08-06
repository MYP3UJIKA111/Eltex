#ifndef CHAT_H
#define CHAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <mqueue.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_MSG_SIZE 256
#define MAX_QUEUE_MSGS 10

#define MSG_PRIORITY_NORMAL 1
#define MSG_PRIORITY_EXIT 10

#define QUEUE_NAME_MAX 64

/* Структура для хранения состояния чата */
typedef struct {
    char queue_name[QUEUE_NAME_MAX];

    char send_queue_name[QUEUE_NAME_MAX];
    char recv_queue_name[QUEUE_NAME_MAX];

    mqd_t send_queue;
    mqd_t recv_queue;

    bool is_creator;

    volatile sig_atomic_t running;
    volatile sig_atomic_t local_exit;

    pthread_t recv_thread;
} chat_context_t;

/* Функции инициализации */
int chat_init(chat_context_t *ctx, const char *base_name);
int chat_create_queues(chat_context_t *ctx);

/* Функции отправки и приема */
void *chat_receive_thread(void *arg);
int chat_send_message(chat_context_t *ctx, const char *message, unsigned int priority);

/* Функции очистки */
void chat_cleanup(chat_context_t *ctx);
void chat_signal_handler(int sig);

/* Вспомогательные функции */
void chat_print_help(void);

#endif /* CHAT_H */