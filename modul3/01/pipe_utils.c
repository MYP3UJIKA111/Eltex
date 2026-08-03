#define _GNU_SOURCE
#include "pipe_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

void init_channel(PipeChannel *ch, const char *name) {
    if (name) {
        ch->mode = MODE_NAMED;
        strncpy(ch->name, name, sizeof(ch->name) - 1);
        ch->name[sizeof(ch->name) - 1] = '\0';
        mkfifo(name, 0666);
    } else {
        ch->mode = MODE_UNNAMED;
        if (pipe(ch->p2c) < 0 || pipe(ch->c2p) < 0) {
            perror("pipe creation failed");
            exit(1);
        }
    }
}

void open_channel_parent(PipeChannel *ch) {
    if (ch->mode == MODE_NAMED) {
        int fd = open(ch->name, O_RDWR);
        if (fd < 0) { perror("parent open named pipe"); exit(1); }
        ch->fd_read = fd;
        ch->fd_write = fd;
    } else {
        ch->fd_read = ch->c2p[0];
        ch->fd_write = ch->p2c[1];
        close(ch->p2c[0]);
        close(ch->c2p[1]);
    }
}

void open_channel_child(PipeChannel *ch) {
    if (ch->mode == MODE_NAMED) {
        int fd = open(ch->name, O_RDWR);
        if (fd < 0) { perror("child open named pipe"); exit(1); }
        ch->fd_read = fd;
        ch->fd_write = fd;
    } else {
        ch->fd_read = ch->p2c[0];
        ch->fd_write = ch->c2p[1];
        close(ch->p2c[1]);
        close(ch->c2p[0]);
    }
}

void close_channel(PipeChannel *ch) {
    close(ch->fd_read);
    if (ch->fd_read != ch->fd_write) {
        close(ch->fd_write);
    }
}

void cleanup_named_pipe(PipeChannel *ch) {
    if (ch->mode == MODE_NAMED) {
        unlink(ch->name);
    }
}