#include "chat.h"

void chat_cleanup(chat_context_t *ctx) {
    if (!ctx) {
        return;
    }

    if (ctx->send_queue != (mqd_t)-1) {
        mq_close(ctx->send_queue);
        ctx->send_queue = (mqd_t)-1;
    }

    if (ctx->recv_queue != (mqd_t)-1) {
        mq_close(ctx->recv_queue);
        ctx->recv_queue = (mqd_t)-1;
    }

    if (ctx->is_creator) {
        printf("Удаление очередей создателем...\n");

        mq_unlink(ctx->recv_queue_name);
        mq_unlink(ctx->send_queue_name);
    }
}

void chat_print_help(void) {
    printf("\n=== P2P Чат ===\n");
    printf("Команды:\n");
    printf("  exit   - завершить работу чата\n");
    printf("  Ctrl-C - отправить сообщение о завершении и выйти\n");
    printf("Примечание: сообщение о завершении отправляется с приоритетом %d\n\n",
           MSG_PRIORITY_EXIT);
}