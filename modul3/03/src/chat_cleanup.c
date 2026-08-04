#include "chat.h"

void chat_cleanup(chat_context_t *ctx) {
    if (!ctx) {
        return;
    }
    
    /* Отправляем сообщение о завершении если еще не отправлено */
    if (ctx->running) {
        chat_send_message(ctx, "EXIT", MSG_PRIORITY_EXIT);
        ctx->running = false;
    }
    
    /* Закрываем очереди */
    if (ctx->send_queue != (mqd_t)-1) {
        mq_close(ctx->send_queue);
        ctx->send_queue = (mqd_t)-1;
    }
    
    if (ctx->recv_queue != (mqd_t)-1) {
        mq_close(ctx->recv_queue);
        ctx->recv_queue = (mqd_t)-1;
    }
    
    /* Если мы создатель - удаляем очереди */
    if (ctx->is_creator) {
        printf("Удаление очередей...\n");
        mq_unlink(ctx->send_queue_name);
        mq_unlink(ctx->recv_queue_name);
    }
}

void chat_print_help(void) {
    printf("\n=== P2P Чат ===\n");
    printf("Команды:\n");
    printf("  exit - завершить работу чата\n");
    printf("  Ctrl-C - отправить сигнал SIGINT и завершить работу\n");
    printf("Примечание: сообщение 'EXIT' с приоритетом %d используется для уведомления о завершении\n\n", MSG_PRIORITY_EXIT);
}