#include "broker.h"

int broker_add_subscriber(broker_context_t *ctx, pid_t pid, const char *topic) {
    if (!ctx || !topic) return -1;
    
    /* Ищем существующего подписчика */
    for (int i = 0; i < ctx->subscriber_count; i++) {
        if (ctx->subscribers[i].pid == pid) {
            /* Добавляем тему, если еще не подписан */
            for (int j = 0; j < ctx->subscribers[i].topic_count; j++) {
                if (strcmp(ctx->subscribers[i].topics[j], topic) == 0) {
                    return 0; /* Уже подписан */
                }
            }
            if (ctx->subscribers[i].topic_count < 5) {
                strncpy(ctx->subscribers[i].topics[ctx->subscribers[i].topic_count], 
                       topic, MAX_TOPIC_SIZE - 1);
                ctx->subscribers[i].topic_count++;
                printf("Подписчик %d подписан на тему '%s'\n", pid, topic);
                return 0;
            }
            return -1;
        }
    }
    
    /* Новый подписчик */
    if (ctx->subscriber_count >= MAX_SUBSCRIBERS) {
        fprintf(stderr, "Достигнуто максимальное количество подписчиков\n");
        return -1;
    }
    
    ctx->subscribers[ctx->subscriber_count].pid = pid;
    strncpy(ctx->subscribers[ctx->subscriber_count].topics[0], topic, MAX_TOPIC_SIZE - 1);
    ctx->subscribers[ctx->subscriber_count].topic_count = 1;
    ctx->subscriber_count++;
    
    printf("Добавлен новый подписчик %d на тему '%s'\n", pid, topic);
    return 0;
}

int broker_remove_subscriber(broker_context_t *ctx, pid_t pid, const char *topic) {
    if (!ctx) return -1;
    
    for (int i = 0; i < ctx->subscriber_count; i++) {
        if (ctx->subscribers[i].pid == pid) {
            /* Удаляем тему */
            for (int j = 0; j < ctx->subscribers[i].topic_count; j++) {
                if (strcmp(ctx->subscribers[i].topics[j], topic) == 0) {
                    /* Сдвигаем остальные темы */
                    for (int k = j; k < ctx->subscribers[i].topic_count - 1; k++) {
                        strcpy(ctx->subscribers[i].topics[k], 
                               ctx->subscribers[i].topics[k + 1]);
                    }
                    ctx->subscribers[i].topic_count--;
                    printf("Подписчик %d отписан от темы '%s'\n", pid, topic);
                    
                    /* Если не осталось тем, удаляем подписчика */
                    if (ctx->subscribers[i].topic_count == 0) {
                        for (int j = i; j < ctx->subscriber_count - 1; j++) {
                            ctx->subscribers[j] = ctx->subscribers[j + 1];
                        }
                        ctx->subscriber_count--;
                    }
                    return 0;
                }
            }
        }
    }
    return -1;
}

int broker_add_publisher(broker_context_t *ctx, pid_t pid) {
    if (!ctx) return -1;
    
    for (int i = 0; i < ctx->publisher_count; i++) {
        if (ctx->publishers[i].pid == pid) {
            return 0; /* Уже существует */
        }
    }
    
    if (ctx->publisher_count >= MAX_PUBLISHERS) {
        fprintf(stderr, "Достигнуто максимальное количество издателей\n");
        return -1;
    }
    
    ctx->publishers[ctx->publisher_count].pid = pid;
    ctx->publisher_count++;
    printf("Добавлен издатель %d\n", pid);
    return 0;
}

void broker_remove_publisher(broker_context_t *ctx, pid_t pid) {
    if (!ctx) return;
    
    for (int i = 0; i < ctx->publisher_count; i++) {
        if (ctx->publishers[i].pid == pid) {
            for (int j = i; j < ctx->publisher_count - 1; j++) {
                ctx->publishers[j] = ctx->publishers[j + 1];
            }
            ctx->publisher_count--;
            printf("Удален издатель %d\n", pid);
            break;
        }
    }
}

void broker_forward_message(broker_context_t *ctx, const char *topic, 
                           const char *payload, pid_t sender_pid) {
    char msg_text[MAX_MSG_SIZE];
    message_t msg;
    
    printf("Пересылка сообщения по теме '%s': %s\n", topic, payload);
    
    /* Пересылаем всем подписчикам на эту тему */
    for (int i = 0; i < ctx->subscriber_count; i++) {
        for (int j = 0; j < ctx->subscribers[i].topic_count; j++) {
            if (strcmp(ctx->subscribers[i].topics[j], topic) == 0) {
                /* Отправляем сообщение подписчику с приоритетом = его pid */
                snprintf(msg_text, sizeof(msg_text), "topic:%s,payload:%s,sender:%d",
                        topic, payload, sender_pid);
                msg.mtype = ctx->subscribers[i].pid;
                strncpy(msg.mtext, msg_text, MAX_MSG_SIZE - 1);
                
                if (msgsnd(ctx->msqid, &msg, strlen(msg.mtext) + 1, IPC_NOWAIT) == -1) {
                    perror("msgsnd (пересылка подписчику)");
                }
            }
        }
    }
}

int broker_process_message(broker_context_t *ctx, message_t *msg) {
    char command[32], pid_str[16], topic[MAX_TOPIC_SIZE];
    pid_t pid;
    char *ptr;
    
    /* Парсим сообщение: command,pid,topic */
    char temp[MAX_MSG_SIZE];
    strncpy(temp, msg->mtext, MAX_MSG_SIZE - 1);
    
    char *cmd = strtok(temp, ",");
    char *pid_str_ptr = strtok(NULL, ",");
    char *topic_ptr = strtok(NULL, ",");
    
    if (!cmd || !pid_str_ptr) {
        fprintf(stderr, "Некорректное сообщение: %s\n", msg->mtext);
        return -1;
    }
    
    pid = atoi(pid_str_ptr);
    
    if (strcmp(cmd, "subscribe") == 0) {
        if (topic_ptr) {
            broker_add_subscriber(ctx, pid, topic_ptr);
            broker_add_publisher(ctx, pid);
        }
    } else if (strcmp(cmd, "unsubscribe") == 0) {
        if (topic_ptr) {
            broker_remove_subscriber(ctx, pid, topic_ptr);
        }
    } else if (strcmp(cmd, "send") == 0) {
        if (topic_ptr) {
            char *payload = strtok(NULL, "");
            broker_add_publisher(ctx, pid);
            broker_forward_message(ctx, topic_ptr, payload ? payload : "", pid);
        }
    } else {
        fprintf(stderr, "Неизвестная команда: %s\n", cmd);
        return -1;
    }
    
    return 0;
}

int broker_run(broker_context_t *ctx) {
    int msqid;
    message_t msg;
    
    printf("Запуск брокера...\n");
    printf("Ключ очереди: %d\n", ctx->key);
    
    /* Пытаемся создать очередь */
    if (queue_create(ctx->key, &msqid) != 0) {
        if (errno == EEXIST) {
            fprintf(stderr, "Ошибка: очередь уже существует. Брокер уже запущен.\n");
            return -1;
        }
        fprintf(stderr, "Ошибка создания очереди\n");
        return -1;
    }
    
    ctx->msqid = msqid;
    printf("Брокер запущен. Очередь создана (msqid=%d)\n", msqid);
    printf("Ожидание сообщений...\n");
    
    /* Основной цикл брокера */
    while (ctx->running) {
        /* Получаем сообщение с приоритетом 1 (от издателей и подписчиков) */
        if (queue_receive(msqid, &msg, MSG_TYPE_BROKER, 0) == 0) {
            broker_process_message(ctx, &msg);
        } else {
            if (errno != EINTR) {
                /* Если очередь удалена или другая ошибка */
                if (errno == EIDRM) {
                    printf("Очередь была удалена\n");
                    break;
                }
                perror("msgrcv");
            }
        }
    }
    
    /* Отправляем SIGINT всем издателям */
    printf("Отправка сигнала завершения издателям...\n");
    for (int i = 0; i < ctx->publisher_count; i++) {
        if (kill(ctx->publishers[i].pid, SIGINT) == 0) {
            printf("Отправлен SIGINT издателю %d\n", ctx->publishers[i].pid);
        }
    }
    
    /* Отправляем SIGINT всем подписчикам */
    printf("Отправка сигнала завершения подписчикам...\n");
    for (int i = 0; i < ctx->subscriber_count; i++) {
        if (kill(ctx->subscribers[i].pid, SIGINT) == 0) {
            printf("Отправлен SIGINT подписчику %d\n", ctx->subscribers[i].pid);
        }
    }
    
    /* Удаляем очередь */
    printf("Удаление очереди...\n");
    queue_destroy(msqid);
    
    printf("Брокер завершил работу\n");
    return 0;
}