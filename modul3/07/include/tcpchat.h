#ifndef TCPCHAT_H
#define TCPCHAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <poll.h>
#include <fcntl.h>

#define TCP_PORT 50001
#define MAX_MSG_SIZE 4096
#define MAX_CLIENTS 10
#define HOSTNAME_MAX 256

// Типы сообщений
#define MSG_JOIN    "JOIN"
#define MSG_LEAVE   "LEAVE"
#define MSG_TEXT    "MSG"
#define MSG_FILE    "FILE"

// Структура клиента на сервере
typedef struct {
    int sockfd;
    char hostname[HOSTNAME_MAX];
    int active;
} client_t;

// Прототипы функций
int run_tcp_server(void);
int run_tcp_client(const char *server_ip);
int send_file_data(int sockfd, const char *filename);

#endif /* TCPCHAT_H */