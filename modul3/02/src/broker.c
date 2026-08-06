#include "broker.h"

static int broker_add_subscriber(broker_context_t *ctx, pid_t pid, const char *topic) {
    if (!ctx || !topic) return -1;

    for (int i = 0; i < ctx->subscriber_count; i++) {
        if (ctx->subscribers[i].pid == pid) {
            for (int j = 0; j < ctx->subscribers[i].topic_count; j++) {
                if (strcmp(ctx->subscribers[i].topics[j], topic) == 0) {
                    return 0;
                }
            }
            if (ctx->subscribers[i].topic_count < MAX_TOPICS_PER_SUB) {
                strncpy(ctx->subscribers[i].topics[ctx->subscribers[i].topic_count],
                        topic, MAX_TOPIC_SIZE - 1);
                ctx->subscribers[i].topics[ctx->subscribers[i].topic_count][MAX_TOPIC_SIZE - 1] = '\0';
                ctx->subscribers[i].topic_count++;
                printf("[Broker] pid %d подписался на '%s'\n", pid, topic);
                return 0;
            }
            return -1;
        }
    }

    if (ctx->subscriber_count >= MAX_SUBSCRIBERS) {
        fprintf(stderr, "[Broker] Слишком много подписчиков\n");
        return -1;
    }

    ctx->subscribers[ctx->subscriber_count].pid = pid;
    strncpy(ctx->subscribers[ctx->subscriber_count].topics[0], topic, MAX_TOPIC_SIZE - 1);
    ctx->subscribers[ctx->subscriber_count].topics[0][MAX_TOPIC_SIZE - 1] = '\0';
    ctx->subscribers[ctx->subscriber_count].topic_count = 1;
    ctx->subscriber_count++;

    printf("[Broker] Новый подписчик pid=%d на тему '%s'\n", pid, topic);
    return 0;
}

static int broker_remove_subscriber(broker_context_t *ctx, pid_t pid, const char *topic) {
    if (!ctx) return -1;

    for (int i = 0; i < ctx->subscriber_count; i++) {
        if (ctx->subscribers[i].pid == pid) {
            for (int j = 0; j < ctx->subscribers[i].topic_count; j++) {
                if (strcmp(ctx->subscribers[i].topics[j], topic) == 0) {
                    for (int k = j; k < ctx->subscribers[i].topic_count - 1; k++) {
                        strcpy(ctx->subscribers[i].topics[k],
                               ctx->subscribers[i].topics[k + 1]);
                    }
                    ctx->subscribers[i].topic_count--;
                    printf("[Broker] pid %d отписался от '%s'\n", pid, topic);

                    if (ctx->subscribers[i].topic_count == 0) {
                        for (int k = i; k < ctx->subscriber_count - 1; k++) {
                            ctx->subscribers[k] = ctx->subscribers[k + 1];
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

static int broker_add_publisher(broker_context_t *ctx, pid_t pid) {
    if (!ctx) return -1;

    for (int i = 0; i < ctx->publisher_count; i++) {
        if (ctx->publishers[i].pid == pid) return 0;
    }

    if (ctx->publisher_count >= MAX_PUBLISHERS) {
        fprintf(stderr, "[Broker] Слишком много издателей\n");
        return -1;
    }

    ctx->publishers[ctx->publisher_count].pid = pid;
    ctx->publisher_count++;
    printf("[Broker] Зарегистрирован издатель pid=%d\n", pid);
    return 0;
}

static void broker_forward_message(broker_context_t *ctx, const char *topic,
                                   const char *full_text) {
    int delivered = 0;

    for (int i = 0; i < ctx->subscriber_count; i++) {
        for (int j = 0; j < ctx->subscribers[i].topic_count; j++) {
            if (strcmp(ctx->subscribers[i].topics[j], topic) == 0) {
                message_t msg;
                msg.mtype = (long)ctx->subscribers[i].pid;
                strncpy(msg.mtext, full_text, MAX_MSG_SIZE - 1);
                msg.mtext[MAX_MSG_SIZE - 1] = '\0';

                if (msgsnd(ctx->msqid, &msg, strlen(msg.mtext) + 1, IPC_NOWAIT) == -1) {
                    if (errno != EAGAIN) {
                        perror("[Broker] msgsnd to subscriber");
                    }
                } else {
                    delivered++;
                }
                break;
            }
        }
    }

    printf("[Broker] Тема '%s': доставлено %d подписчикам\n", topic, delivered);
}

static int broker_process_message(broker_context_t *ctx, message_t *msg) {
    char temp[MAX_MSG_SIZE];
    strncpy(temp, msg->mtext, MAX_MSG_SIZE - 1);
    temp[MAX_MSG_SIZE - 1] = '\0';

    char *first_comma = strchr(temp, ',');
    if (!first_comma) return -1;
    *first_comma = '\0';
    char *cmd = temp;
    char *rest = first_comma + 1;

    char *second_comma = strchr(rest, ',');
    if (!second_comma) return -1;
    *second_comma = '\0';
    char *pid_str = rest;
    char *topic_rest = second_comma + 1;

    pid_t pid = (pid_t)atoi(pid_str);
    if (pid <= 0) return -1;

    if (strcmp(cmd, "subscribe") == 0) {
        broker_add_subscriber(ctx, pid, topic_rest);
    } else if (strcmp(cmd, "unsubscribe") == 0) {
        broker_remove_subscriber(ctx, pid, topic_rest);
    } else if (strcmp(cmd, "send") == 0) {
        char *third_comma = strchr(topic_rest, ',');
        char *topic;
        const char *payload;

        if (third_comma) {
            *third_comma = '\0';
            topic = topic_rest;
            payload = third_comma + 1;
        } else {
            topic = topic_rest;
            payload = "";
        }

        broker_add_publisher(ctx, pid);
        broker_forward_message(ctx, topic, msg->mtext);
        (void)payload;
    } else {
        fprintf(stderr, "[Broker] Неизвестная команда: %s\n", cmd);
        return -1;
    }

    return 0;
}

static void broker_send_stop_signals(broker_context_t *ctx) {
    printf("[Broker] Рассылка SIGINT всем клиентам...\n");

    for (int i = 0; i < ctx->publisher_count; i++) {
        if (kill(ctx->publishers[i].pid, SIGINT) == -1) {
            if (errno != ESRCH) {
                perror("[Broker] kill publisher");
            }
        }
    }

    for (int i = 0; i < ctx->subscriber_count; i++) {
        bool already_sent = false;
        for (int j = 0; j < ctx->publisher_count; j++) {
            if (ctx->publishers[j].pid == ctx->subscribers[i].pid) {
                already_sent = true;
                break;
            }
        }
        if (!already_sent) {
            if (kill(ctx->subscribers[i].pid, SIGINT) == -1 && errno != ESRCH) {
                perror("[Broker] kill subscriber");
            }
        }
    }
}

static void broker_wait_queue_empty(int msqid) {
    for (int waited = 0; waited < SHUTDOWN_TIMEOUT_MS; waited += 100) {
        struct msqid_ds ds;
        if (msgctl(msqid, IPC_STAT, &ds) == -1) break;
        if (ds.msg_qnum == 0) break;

        message_t msg;
        while (msgrcv(msqid, &msg, MAX_MSG_SIZE, 0, IPC_NOWAIT) >= 0) {
        }
        usleep(100000);
    }
}

int broker_run(broker_context_t *ctx) {
    printf("[Broker] Запуск (pid=%d, key=0x%X)...\n", getpid(), (unsigned)ctx->key);

    if (queue_create(ctx->key, &ctx->msqid) != 0) {
        if (errno == EEXIST) {
            fprintf(stderr, "[Broker] Очередь уже существует. Другой брокер уже запущен.\n");
        } else {
            perror("[Broker] msgget");
        }
        return -1;
    }

    printf("[Broker] Очередь создана, msqid=%d\n", ctx->msqid);

    while (ctx->running) {
        message_t msg;
        ssize_t bytes = msgrcv(ctx->msqid, &msg, MAX_MSG_SIZE,
                               MSG_TYPE_BROKER, 0);

        if (bytes >= 0) {
            msg.mtext[bytes] = '\0';
            broker_process_message(ctx, &msg);
        } else {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EIDRM) {
                printf("[Broker] Очередь удалена извне\n");
                break;
            }
            perror("[Broker] msgrcv");
            break;
        }
    }

    printf("[Broker] Завершение работы...\n");

    broker_send_stop_signals(ctx);

    broker_wait_queue_empty(ctx->msqid);

    queue_destroy(ctx->msqid);
    printf("[Broker] Очередь удалена. Работа завершена.\n");

    return 0;
}