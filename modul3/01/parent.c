#define _GNU_SOURCE
#include "parent.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void run_parent(PipeChannel *ch, int argc, char **argv, int start_idx) {
    ReadyMsg rdy;
    CtrlMsg msg;

    for (int i = start_idx; i < argc; i++) {
        recv_msg(ch->fd_read, &rdy, sizeof(ReadyMsg));
        
        int file_fd = open(argv[i], O_RDONLY);
        if (file_fd < 0) {
            fprintf(stderr, "Ошибка: файл '%s' не существует или недоступен.\n", argv[i]);
            msg.type = MSG_SKIP;
            msg.size = 0;
            strncpy(msg.filename, argv[i], MAX_FILENAME - 1);
            send_msg(ch->fd_write, &msg, sizeof(CtrlMsg));
            continue;
        }

        struct stat st;
        fstat(file_fd, &st);

        msg.type = MSG_FILE;
        msg.size = st.st_size;
        strncpy(msg.filename, argv[i], MAX_FILENAME - 1);
        
        send_msg(ch->fd_write, &msg, sizeof(CtrlMsg));
        send_file_data(ch->fd_write, file_fd, msg.size);
        close(file_fd);
    }

    recv_msg(ch->fd_read, &rdy, sizeof(ReadyMsg));
    
    msg.type = MSG_DONE;
    msg.size = 0;
    msg.filename[0] = '\0';
    send_msg(ch->fd_write, &msg, sizeof(CtrlMsg));

    int status;
    wait(&status);
}