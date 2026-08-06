#include "chat.h"

static int chat_receive_message(chat_context_t *ctx,
                                char *buffer,
                                size_t buffer_size,
                                unsigned int *priority) {
    if (!ctx || !buffer || buffer_size == 0) {
        return -1;
    }

    ssize_t bytes = mq_receive(ctx->recv_queue,
                               buffer,
                               buffer_size,
                               priority);

    if (bytes == -1) {
        return -1;
    }

    if (bytes >= (ssize_t)buffer_size) {
        bytes = (ssize_t)buffer_size - 1;
    }

    buffer[bytes] = '\0';

    return 0;
}

void *chat_receive_thread(void *arg) {
    chat_context_t *ctx = (chat_context_t *)arg;

    char buffer[MAX_MSG_SIZE + 1];
    unsigned int priority = 0;

    struct mq_attr attr;

    
    if (mq_getattr(ctx->recv_queue, &attr) == 0) {
        attr.mq_flags |= O_NONBLOCK;
        mq_setattr(ctx->recv_queue, &attr, NULL);
    }

    while (ctx->running) {
        if (chat_receive_message(ctx, buffer, sizeof(buffer), &priority) == 0) {

            if (priority == MSG_PRIORITY_EXIT) {
                printf("\n[Собеседник завершил работу]\n");
                fflush(stdout);

                ctx->running = 0;
                break;
            }

            printf("\n[Собеседник] %s\n", buffer);
            fflush(stdout);
        } else {
            if (errno == EAGAIN) {
                /* Сообщений пока нет */
                usleep(100000);
                continue;
            }

            if (errno == EINTR) {
                continue;
            }

            
            perror("mq_receive");
            ctx->running = 0;
            break;
        }
    }

    return NULL;
}

int chat_send_message(chat_context_t *ctx, const char *message, unsigned int priority) {
    if (!ctx || !message) {
        return -1;
    }

    if (ctx->send_queue == (mqd_t)-1) {
        fprintf(stderr, "Очередь отправки не инициализирована\n");
        return -1;
    }

    char buf[MAX_MSG_SIZE];

    size_t len = strlen(message);

    
    if (len >= MAX_MSG_SIZE) {
        len = MAX_MSG_SIZE - 1;
    }

    memcpy(buf, message, len);
    buf[len] = '\0';

    ++len;

    for (int attempt = 0; attempt < 50; ++attempt) {
        if (mq_send(ctx->send_queue, buf, len, priority) == 0) {
            return 0;
        }

        if (errno == EAGAIN) {
            usleep(20000);
            continue;
        }

        if (errno == EINTR) {
            continue;
        }

        return -1;
    }

    errno = EAGAIN;
    return -1;
}