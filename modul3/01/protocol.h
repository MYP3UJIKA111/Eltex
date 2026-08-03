#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <sys/types.h> // Для off_t, ssize_t

#define MAX_FILENAME 1024
#define BLOCK_SIZE 1024

#define MSG_READY 0x11
#define MSG_FILE  0x22
#define MSG_SKIP  0x33
#define MSG_DONE  0x44

typedef struct {
    int type;
    off_t size;
    char filename[MAX_FILENAME];
} CtrlMsg;

typedef struct {
    int status;
} ReadyMsg;

void send_msg(int fd, const void *msg, size_t size);
void recv_msg(int fd, void *msg, size_t size);
void send_file_data(int write_fd, int file_fd, off_t size);
void recv_file_data(int read_fd, int file_fd, off_t size);

#endif