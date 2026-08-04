#include "chat.h"

int chat_receive_message(chat_context_t *ctx, char *buffer, unsigned int *priority) {
    if (!ctx || !buffer) {
        return -1;
    }
    
    ssize_t bytes;
    bytes = mq_receive(ctx->recv_queue, buffer, MAX_MSG_SIZE, priority);
    
    if (bytes == -1) {
        return -1;
    }
    
    buffer[bytes] = '\0';
    return 0;
}

void* chat_receive_thread(void *arg) {
    chat_context_t *ctx = (chat_context_t*)arg;
    char buffer[MAX_MSG_SIZE];
    unsigned int priority;
    struct mq_attr attr;
    int ret;
    
    /* Устанавливаем неблокирующий режим */
    if (mq_getattr(ctx->recv_queue, &attr) == 0) {
        attr.mq_flags |= O_NONBLOCK;
        mq_setattr(ctx->recv_queue, &attr, NULL);
    }
    
    while (ctx->running) {
        ret = chat_receive_message(ctx, buffer, &priority);
        
        if (ret == 0) {
            /* Проверяем сообщение о завершении */
            if (priority == MSG_PRIORITY_EXIT || strcmp(buffer, "EXIT") == 0) {
                printf("\n[Собеседник завершил работу]\n");
                ctx->running = false;
                break;
            }
            
            /* Выводим полученное сообщение */
            printf("\n[Получено] %s\n", buffer);
            printf("> ");
            fflush(stdout);
        } else if (errno != EAGAIN) {
            /* Реальная ошибка (не просто отсутствие сообщений) */
            if (errno != EINTR) {
                perror("mq_receive");
                break;
            }
        }
        
        usleep(100000); /* 100 мс */
    }
    
    return NULL;
}