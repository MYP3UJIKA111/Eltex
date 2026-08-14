#ifndef UDPCHAT_H
#define UDPCHAT_H

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

#define CHAT_PORT 50000
#define MAX_MSG_SIZE 1024
#define HOSTNAME_MAX 256

#define MSG_JOIN  "JOIN"
#define MSG_LEAVE "LEAVE"
#define MSG_TEXT  "MSG"

// common.c
int send_broadcast(int sockfd, const char *message);
int parse_message(const char *raw, char *type, int *pid,
                  char *hostname, size_t hostname_sz,
                  char *text, size_t text_sz);

// chat.c
int run_chat(void);

#endif /* UDPCHAT_H */