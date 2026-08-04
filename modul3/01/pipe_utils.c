#define _GNU_SOURCE
#include "pipe_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

void init_channel(PipeChannel *ch, const char *name) {
    if (name) {
        ch->mode = MODE_NAMED;
        
        snprintf(ch->name_to_parent, sizeof(ch->name_to_parent), "%s_to_parent", name);
        snprintf(ch->name_to_child, sizeof(ch->name_to_child), "%s_to_child", name);
        
        unlink(ch->name_to_parent);
        unlink(ch->name_to_child);
        
        if (mkfifo(ch->name_to_parent, 0666) < 0) {
            perror("mkfifo to_parent failed");
            exit(1);
        }
        if (mkfifo(ch->name_to_child, 0666) < 0) {
            perror("mkfifo to_child failed");
            exit(1);
        }
        
        ch->fd_read = -1;
        ch->fd_write = -1;
    } else {
        ch->mode = MODE_UNNAMED;
        if (pipe(ch->p2c) < 0 || pipe(ch->c2p) < 0) {
            perror("pipe creation failed");
            exit(1);
        }
        ch->fd_read = -1;
        ch->fd_write = -1;
    }
}

void open_channel_parent(PipeChannel *ch) {
    if (ch->mode == MODE_NAMED) {
        ch->fd_read = open(ch->name_to_parent, O_RDONLY | O_NONBLOCK);
        if (ch->fd_read < 0) {
            perror("parent open read fifo");
            exit(1);
        }
        
        ch->fd_write = open(ch->name_to_child, O_WRONLY);
        if (ch->fd_write < 0) {
            perror("parent open write fifo");
            exit(1);
        }
        
        int flags = fcntl(ch->fd_read, F_GETFL, 0);
        fcntl(ch->fd_read, F_SETFL, flags & ~O_NONBLOCK);
    } else {
        ch->fd_read = ch->c2p[0];
        ch->fd_write = ch->p2c[1];
        
        close(ch->p2c[0]);
        close(ch->c2p[1]);
    }
}

void open_channel_child(PipeChannel *ch) {
    if (ch->mode == MODE_NAMED) {
        ch->fd_write = open(ch->name_to_parent, O_WRONLY);
        if (ch->fd_write < 0) {
            perror("child open write fifo");
            exit(1);
        }
        
        ch->fd_read = open(ch->name_to_child, O_RDONLY);
        if (ch->fd_read < 0) {
            perror("child open read fifo");
            exit(1);
        }
    } else {
        ch->fd_read = ch->p2c[0];
        ch->fd_write = ch->c2p[1];
        
        close(ch->p2c[1]);
        close(ch->c2p[0]);
    }
}

void close_channel(PipeChannel *ch) {
    if (ch->fd_read >= 0) {
        close(ch->fd_read);
        ch->fd_read = -1;
    }
    if (ch->fd_write >= 0 && ch->fd_write != ch->fd_read) {
        close(ch->fd_write);
        ch->fd_write = -1;
    }
}

void cleanup_named_pipe(PipeChannel *ch) {
    if (ch->mode == MODE_NAMED) {
        unlink(ch->name_to_parent);
        unlink(ch->name_to_child);
    }
}