#define _GNU_SOURCE
#include "child.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

void run_child(PipeChannel *ch) {
    ReadyMsg rdy;
    CtrlMsg msg;
    
    rdy.status = MSG_READY;

    while (1) {
        send_msg(ch->fd_write, &rdy, sizeof(ReadyMsg));
        recv_msg(ch->fd_read, &msg, sizeof(CtrlMsg));

        if (msg.type == MSG_DONE) break;
        if (msg.type == MSG_SKIP) continue;

        char copy_filename[MAX_FILENAME + 10];
        snprintf(copy_filename, sizeof(copy_filename), "%s.copy", msg.filename);

        int file_fd = open(copy_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (file_fd < 0) {
            perror("child: create copy failed");
            exit(1);
        }

        recv_file_data(ch->fd_read, file_fd, msg.size);
        close(file_fd);
    }
    exit(0);
}