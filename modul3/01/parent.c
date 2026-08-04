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
#include <errno.h>

void run_parent(PipeChannel *ch, int argc, char **argv, int start_idx) {
    ReadyMsg rdy;
    CtrlMsg msg;
    int file_count = 0;

    for (int i = start_idx; i < argc; i++) {
        // Ждем готовность дочернего процесса
        recv_msg(ch->fd_read, &rdy, sizeof(ReadyMsg));
        
        // Пытаемся открыть файл
        int file_fd = open(argv[i], O_RDONLY);
        if (file_fd < 0) {
            fprintf(stderr, "Ошибка: файл '%s' не существует или недоступен: %s\n", 
                    argv[i], strerror(errno));
            msg.type = MSG_SKIP;
            msg.size = 0;
            strncpy(msg.filename, argv[i], MAX_FILENAME - 1);
            msg.filename[MAX_FILENAME - 1] = '\0';
            send_msg(ch->fd_write, &msg, sizeof(CtrlMsg));
            continue;
        }

        // Получаем информацию о файле
        struct stat st;
        if (fstat(file_fd, &st) < 0) {
            perror("fstat failed");
            close(file_fd);
            continue;
        }

        // Отправляем информацию о файле
        msg.type = MSG_FILE;
        msg.size = st.st_size;
        strncpy(msg.filename, argv[i], MAX_FILENAME - 1);
        msg.filename[MAX_FILENAME - 1] = '\0';
        
        send_msg(ch->fd_write, &msg, sizeof(CtrlMsg));
        
        // Отправляем содержимое файла
        send_file_data(ch->fd_write, file_fd, msg.size);
        close(file_fd);
        file_count++;
    }

    // Ждем готовность дочернего процесса перед завершением
    recv_msg(ch->fd_read, &rdy, sizeof(ReadyMsg));
    
    // Отправляем сигнал завершения
    msg.type = MSG_DONE;
    msg.size = 0;
    msg.filename[0] = '\0';
    send_msg(ch->fd_write, &msg, sizeof(CtrlMsg));

    // Ожидаем завершения дочернего процесса
    int status;
    wait(&status);
    
    if (WIFEXITED(status)) {
        printf("Дочерний процесс завершен с кодом: %d\n", WEXITSTATUS(status));
    }
}