#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> // Для pid_t
#include "pipe_utils.h"
#include "parent.h"
#include "child.h"

void print_usage(const char *prog) {
    fprintf(stderr, "Использование: %s [-p pipe_name] file1 [file2 ...]\n", prog);
}

int main(int argc, char **argv) {
    char *pipe_name = NULL;
    int opt;
    
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        if (opt == 'p') {
            pipe_name = optarg;
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Ошибка: Не указаны файлы для копирования.\n");
        print_usage(argv[0]);
        return 1;
    }

    PipeChannel ch;
    init_channel(&ch, pipe_name);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid > 0) {
        open_channel_parent(&ch);
        run_parent(&ch, argc, argv, optind);
        close_channel(&ch);
        cleanup_named_pipe(&ch);
    } else {
        open_channel_child(&ch);
        run_child(&ch);
    }

    return 0;
}