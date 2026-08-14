#include "udpchat.h"

int send_broadcast(int sockfd, const char *message) {
    struct sockaddr_in broadcast_addr;

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(CHAT_PORT);
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    ssize_t sent = sendto(sockfd, message, strlen(message), 0,
                          (struct sockaddr *)&broadcast_addr,
                          sizeof(broadcast_addr));

    if (sent < 0) {
        perror("sendto");
        return -1;
    }
    return 0;
}

// Парсинг сообщения. text копируется в предоставленный буфер.
int parse_message(const char *raw, char *type, int *pid,
                  char *hostname, size_t hostname_sz,
                  char *text, size_t text_sz) {
    if (!raw || !type || !pid || !hostname) return -1;

    char buf[MAX_MSG_SIZE];
    strncpy(buf, raw, MAX_MSG_SIZE - 1);
    buf[MAX_MSG_SIZE - 1] = '\0';

    // 1. Тип (до первой ':')
    char *first_colon = strchr(buf, ':');
    if (!first_colon) return -1;
    *first_colon = '\0';
    strncpy(type, buf, 16);
    type[15] = '\0';

    char *rest = first_colon + 1;

    // 2. PID (до второй ':')
    char *second_colon = strchr(rest, ':');
    if (!second_colon) return -1;
    *second_colon = '\0';
    *pid = atoi(rest);
    if (*pid <= 0) return -1;

    char *host_start = second_colon + 1;

    // 3. Hostname и (опционально) текст
    if (strcmp(type, MSG_TEXT) == 0) {
        char *third_colon = strchr(host_start, ':');
        if (!third_colon) return -1;
        *third_colon = '\0';
        strncpy(hostname, host_start, hostname_sz - 1);
        hostname[hostname_sz - 1] = '\0';
        if (text && text_sz > 0) {
            strncpy(text, third_colon + 1, text_sz - 1);
            text[text_sz - 1] = '\0';
        }
    } else {
        strncpy(hostname, host_start, hostname_sz - 1);
        hostname[hostname_sz - 1] = '\0';
        if (text && text_sz > 0) text[0] = '\0';
    }

    return 0;
}