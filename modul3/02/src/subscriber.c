#include "broker.h"

static bool subscriber_running = true;

void subscriber_signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        subscriber_running = false;
        printf("\nПодписчик завершает работу...\n");
    }
}

int subscriber_subscribe(int msqid, pid_t pid, const char *topic) {
    char msg_text[MAX_MSG_SIZE];
    
    snprintf(msg_text, sizeof(msg_text), "subscribe,%d,%s", pid, topic);
    return queue_send(msqid, 1, msg_text);
}

int subscriber_unsubscribe(int msqid, pid_t pid, const char *topic) {
    char msg_text[MAX_MSG_SIZE];
    
    snprintf(msg_text, sizeof(msg_text), "unsubscribe,%d,%s", pid, topic);
    return queue_send(msqid, 1, msg_text);
}

int subscriber_run(key_t key, char *topics[]) {
    int msqid;
    pid_t pid = getpid();
    message_t msg;
    int topic_count = 0;
    struct sigaction sa;
    
    /* Подсчитываем количество тем */
    while (topics[topic_count] != NULL) {
        topic_count++;
    }
    
    if (topic_count == 0) {
        fprintf(stderr, "Не указаны темы для подписки\n");
        return -1;
    }
    
    printf("Запуск подписчика (PID=%d)\n", pid);
    for (int i = 0; i < topic_count; i++) {
        printf("  Подписка на тему: %s\n", topics[i]);
    }
    
    /* Устанавливаем обработчик сигналов */
    sa.sa_handler = subscriber_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    /* Открываем очередь */
    if (queue_open(key, &msqid) != 0) {
        fprintf(stderr, "Ошибка открытия очереди. Брокер не запущен.\n");
        return -1;
    }
    
    /* Подписываемся на все указанные темы */
    for (int i = 0; i < topic_count; i++) {
        if (subscriber_subscribe(msqid, pid, topics[i]) != 0) {
            fprintf(stderr, "Ошибка подписки на тему: %s\n", topics[i]);
        }
    }
    
    printf("Подписчик запущен. Ожидание сообщений...\n");
    printf("(Нажмите Ctrl+C для выхода)\n");
    
    /* Основной цикл - читаем сообщения с приоритетом = наш pid */
    while (subscriber_running) {
        if (queue_receive(msqid, &msg, pid, IPC_NOWAIT) == 0) {
            /* Парсим сообщение: topic:xxx,payload:yyy,sender:zzz */
            char topic[MAX_TOPIC_SIZE];
            char payload[MAX_MSG_SIZE];
            char sender[16];
            
            char *topic_ptr = strstr(msg.mtext, "topic:");
            char *payload_ptr = strstr(msg.mtext, "payload:");
            char *sender_ptr = strstr(msg.mtext, "sender:");
            
            if (topic_ptr && payload_ptr && sender_ptr) {
                sscanf(topic_ptr, "topic:%[^,]", topic);
                sscanf(payload_ptr, "payload:%[^,]", payload);
                sscanf(sender_ptr, "sender:%s", sender);
                
                printf("\n[%s] Получено сообщение\n", topics[0]);
                printf("  Тема: %s\n", topic);
                printf("  Содержимое: %s\n", payload);
                printf("  От издателя: %s\n", sender);
                printf("> ");
                fflush(stdout);
            }
        } else {
            if (errno != ENOMSG && errno != EINTR) {
                if (errno == EIDRM) {
                    printf("\nОчередь была удалена брокером\n");
                    break;
                }
                perror("msgrcv");
            }
        }
        usleep(100000); /* 100 мс */
    }
    
    /* Отписываемся от всех тем */
    printf("Отписка от тем...\n");
    for (int i = 0; i < topic_count; i++) {
        subscriber_unsubscribe(msqid, pid, topics[i]);
    }
    
    printf("Подписчик завершил работу\n");
    return 0;
}