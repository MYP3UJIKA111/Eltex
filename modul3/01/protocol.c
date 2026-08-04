#define _GNU_SOURCE
#include "protocol.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void send_msg(int fd, const void *msg, size_t size) {
    size_t total = 0;
    while (total < size) {
        ssize_t w = write(fd, (const char*)msg + total, size - total);
        if (w <= 0) {
            perror("send_msg write error");
            exit(1);
        }
        total += w;
    }
}

void recv_msg(int fd, void *msg, size_t size) {
    size_t total = 0;
    while (total < size) {
        ssize_t r = read(fd, (char*)msg + total, size - total);
        if (r <= 0) {
            perror("recv_msg read error");
            exit(1);
        }
        total += r;
    }
}

void send_file_data(int write_fd, int file_fd, off_t size) {
    char buffer[BLOCK_SIZE];
    off_t remaining = size;
    while (remaining > 0) {
        size_t to_read = (remaining < BLOCK_SIZE) ? remaining : BLOCK_SIZE;
        ssize_t r = read(file_fd, buffer, to_read);
        if (r <= 0) break;
        
        ssize_t w = write(write_fd, buffer, r);
        if (w <= 0) {
            perror("send_file_data write error");
            break;
        }
        remaining -= r;
    }
}

void recv_file_data(int read_fd, int file_fd, off_t size) {
    char buffer[BLOCK_SIZE];
    off_t remaining = size;
    while (remaining > 0) {
        size_t to_read = (remaining < BLOCK_SIZE) ? remaining : BLOCK_SIZE;
        ssize_t r = read(read_fd, buffer, to_read);
        if (r <= 0) {
            perror("recv_file_data read error");
            break;
        }
        
        ssize_t w = write(file_fd, buffer, r);
        if (w <= 0) {
            perror("recv_file_data write error");
            break;
        }
        
        remaining -= r;
    }
}