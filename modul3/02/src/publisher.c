#include "broker.h"

static bool publisher_running = true;

void publisher_signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        publisher_running = false;
        printf("\nИздатель завершает работу...\n");
    }
}

int publisher_send_message(int msqid, pid_t pid, const char *topic, const char *payload) {
    char msg_text[MAX_MSG_SIZE];
    
    /* Формируем сообщение: send,pid,topic,payload */
    snprintf(msg_text, sizeof(msg_text), "send,%d,%s,%s", pid, topic, payload);
    
    return queue_send(msqid, 1, msg_text);
}

int publisher_run(key_t key, const char *topic) {
    int msqid;
    char input[MAX_MSG_SIZE];
    char payload[MAX_MSG_SIZE];
    pid_t pid = getpid();
    struct sigaction sa;
    
    printf("Запуск издателя (PID=%d)\n", pid);
    printf("Тема: %s\n", topic);
    
    /* Устанавливаем обработчик сигналов */
    sa.sa_handler = publisher_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    /* Открываем очередь */
    if (queue_open(key, &msqid) != 0) {
        fprintf(stderr, "Ошибка открытия очереди. Брокер не запущен.\n");
        return -1;
    }
    
    printf("Издатель запущен. Введите сообщения для публикации (или 'exit' для выхода):\n");
    printf("> ");
    fflush(stdout);
    
    while (publisher_running) {
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        /* Удаляем символ новой строки */
        input[strcspn(input, "\n")] = '\0';
        
        if (strcmp(input, "exit") == 0) {
            break;
        }
        
        if (strlen(input) > 0) {
            strncpy(payload, input, MAX_MSG_SIZE - 1);
            if (publisher_send_message(msqid, pid, topic, payload) == 0) {
                printf("Сообщение отправлено: %s\n", payload);
            } else {
                /* Проверяем, не удалена ли очередь */
                if (errno == EIDRM) {
                    printf("Очередь была удалена брокером\n");
                    break;
                }
                fprintf(stderr, "Ошибка отправки сообщения\n");
            }
        }
        printf("> ");
        fflush(stdout);
    }
    
    printf("Издатель завершил работу\n");
    return 0;
}