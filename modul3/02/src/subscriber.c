#include "broker.h"

static volatile sig_atomic_t subscriber_running = 1;

static void subscriber_signal_handler(int sig) {
    (void)sig;
    subscriber_running = 0;
}

static int send_subscription(int msqid, pid_t pid, const char *topic, bool subscribe) {
    char msg_text[MAX_MSG_SIZE];
    snprintf(msg_text, sizeof(msg_text), "%s,%d,%s",
             subscribe ? "subscribe" : "unsubscribe",
             (int)pid, topic);
    return queue_send(msqid, MSG_TYPE_BROKER, msg_text);
}

int subscriber_run(key_t key, char *topics[], int topic_count) {
    pid_t pid = getpid();
    int msqid;

    printf("[Subscriber] Запуск (pid=%d)\n", pid);
    printf("[Subscriber] Темы:");
    for (int i = 0; i < topic_count; i++) printf(" '%s'", topics[i]);
    printf("\n");

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = subscriber_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (queue_open(key, &msqid) != 0) {
        fprintf(stderr, "[Subscriber] Очередь недоступна. Брокер не запущен.\n");
        return -1;
    }

    /* Подписываемся на все темы */
    for (int i = 0; i < topic_count; i++) {
        if (send_subscription(msqid, pid, topics[i], true) != 0) {
            perror("[Subscriber] subscribe");
            return -1;
        }
    }

    /* Основной цикл: читаем сообщения с типом = свой pid */
    while (subscriber_running) {
        message_t msg;
        
        /* Используем msgrcv напрямую вместо queue_receive */
        ssize_t bytes = msgrcv(msqid, &msg, MAX_MSG_SIZE,
                               (long)pid, IPC_NOWAIT);

        if (bytes >= 0) {
            msg.mtext[bytes] = '\0';
            printf("\n[Получено] %s\n", msg.mtext);
            fflush(stdout);
        } else if (errno == ENOMSG) {
            usleep(100000); /* 100 ms, если сообщений нет */
        } else if (errno == EINTR) {
            continue; /* Прервано сигналом, пробуем снова */
        } else if (errno == EIDRM || errno == EINVAL) {
            printf("\n[Subscriber] Очередь удалена брокером\n");
            return 0;
        } else {
            perror("[Subscriber] msgrcv");
            break;
        }
    }

    /* При завершении отписываемся от всех тем */
    printf("[Subscriber] Отправка unsubscribe...\n");
    for (int i = 0; i < topic_count; i++) {
        /* Игнорируем ошибки, так как брокер мог уже удалить очередь */
        (void)send_subscription(msqid, pid, topics[i], false);
    }

    printf("[Subscriber] Завершил работу\n");
    return 0;
}