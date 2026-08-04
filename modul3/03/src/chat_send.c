#include "chat.h"

int chat_send_message(chat_context_t *ctx, const char *message, unsigned int priority) {
    if (!ctx || !message) {
        return -1;
    }
    
    if (ctx->send_queue == (mqd_t)-1) {
        fprintf(stderr, "Очередь отправки не инициализирована\n");
        return -1;
    }
    
    if (mq_send(ctx->send_queue, message, strlen(message) + 1, priority) == -1) {
        perror("mq_send");
        return -1;
    }
    
    return 0;
}

void* chat_send_thread(void *arg) {
    chat_context_t *ctx = (chat_context_t*)arg;
    char buffer[MAX_MSG_SIZE];
    
    while (ctx->running) {
        /* Читаем ввод с консоли */
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }
        
        buffer[strcspn(buffer, "\n")] = '\0';
        
        if (strlen(buffer) == 0) {
            continue;
        }
        
        /* Проверка на выход */
        if (strcmp(buffer, "exit") == 0) {
            ctx->running = false;
            chat_send_message(ctx, "EXIT", MSG_PRIORITY_EXIT);
            break;
        }
        
        chat_send_message(ctx, buffer, MSG_PRIORITY_NORMAL);
    }
    
    return NULL;
}