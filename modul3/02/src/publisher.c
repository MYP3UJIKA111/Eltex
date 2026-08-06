#include "broker.h"
#include <sys/select.h>

static volatile sig_atomic_t publisher_running = 1;

static void publisher_signal_handler(int sig) {
    (void)sig;
    publisher_running = 0;
}

int publisher_run(key_t key, const char *topic) {
    pid_t pid = getpid();
    int msqid;

    printf("[Publisher] Запуск (pid=%d, topic='%s')\n", pid, topic);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = publisher_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (queue_open(key, &msqid) != 0) {
        fprintf(stderr, "[Publisher] Очередь недоступна. Брокер не запущен.\n");
        return -1;
    }

    printf("[Publisher] Введите сообщения (или 'exit', Ctrl+C, Ctrl+D):\n");

    char input[MAX_MSG_SIZE];

    while (publisher_running) {
        /* Проверяем, жива ли очередь */
        struct msqid_ds ds;
        if (msgctl(msqid, IPC_STAT, &ds) == -1) {
            if (errno == EIDRM || errno == EINVAL) {
                printf("\n[Publisher] Очередь удалена брокером\n");
                break;
            }
        }

        /* Используем select(), чтобы не блокироваться на fgets */
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);

        struct timeval tv = {0, 100000}; /* 100 ms */

        int sel = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
        if (sel == -1) {
            if (errno == EINTR) continue;
            break;
        }
        if (sel == 0) continue;

        if (!FD_ISSET(STDIN_FILENO, &rfds)) continue;

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) break;
        if (input[0] == '\0') continue;

        /* Формируем сообщение: send,<pid>,<topic>,<payload> */
        char msg_text[MAX_MSG_SIZE];
        snprintf(msg_text, sizeof(msg_text), "send,%d,%s,%s",
                 (int)pid, topic, input);

        if (queue_send(msqid, MSG_TYPE_BROKER, msg_text) != 0) {
            if (errno == EIDRM || errno == EINVAL) {
                printf("\n[Publisher] Очередь удалена\n");
            } else {
                perror("[Publisher] msgsnd");
            }
            break;
        }
    }

    printf("[Publisher] Завершил работу\n");
    return 0;
}