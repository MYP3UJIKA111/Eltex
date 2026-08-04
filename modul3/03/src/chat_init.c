#include "chat.h"

int chat_init(chat_context_t *ctx, const char *base_name) {
    if (!ctx || !base_name) {
        return -1;
    }
    
    memset(ctx, 0, sizeof(chat_context_t));
    ctx->running = true;
    ctx->is_creator = false;
    ctx->send_queue = (mqd_t)-1;
    ctx->recv_queue = (mqd_t)-1;
    
    /* Формируем имена очередей с проверкой длины */
    snprintf(ctx->queue_name, sizeof(ctx->queue_name), "/%s", base_name);
    
    /* Убеждаемся, что имена не превышают лимит */
    if (strlen(ctx->queue_name) + 2 >= QUEUE_NAME_MAX) {
        fprintf(stderr, "Имя очереди слишком длинное\n");
        return -1;
    }
    
    snprintf(ctx->send_queue_name, sizeof(ctx->send_queue_name), "%s_1", ctx->queue_name);
    snprintf(ctx->recv_queue_name, sizeof(ctx->recv_queue_name), "%s_2", ctx->queue_name);
    
    return 0;
}

int chat_create_queues(chat_context_t *ctx) {
    struct mq_attr attr;
    mqd_t q1, q2;
    
    attr.mq_maxmsg = MAX_QUEUE_MSGS;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_flags = 0;
    
    /* Пытаемся создать первую очередь */
    q1 = mq_open(ctx->send_queue_name, O_CREAT | O_RDWR | O_EXCL, 0600, &attr);
    
    if (q1 != (mqd_t)-1) {
        /* Первая очередь создана успешно - значит мы создатель */
        ctx->is_creator = true;
        ctx->send_queue = q1;
        
        /* Создаем вторую очередь */
        q2 = mq_open(ctx->recv_queue_name, O_CREAT | O_RDWR | O_EXCL, 0600, &attr);
        if (q2 == (mqd_t)-1) {
            perror("mq_open (создание второй очереди)");
            mq_close(q1);
            mq_unlink(ctx->send_queue_name);
            ctx->send_queue = (mqd_t)-1;
            return -1;
        }
        ctx->recv_queue = q2;
        
        printf("Созданы новые очереди (создатель)\n");
        printf("  Отправка через: %s\n", ctx->send_queue_name);
        printf("  Прием через: %s\n", ctx->recv_queue_name);
    } else {
        /* Очереди уже существуют - открываем их */
        if (errno != EEXIST) {
            perror("mq_open");
            return -1;
        }
        
        ctx->is_creator = false;
        
        /* Для существующих очередей: первая для отправки, вторая для приема */
        ctx->send_queue = mq_open(ctx->send_queue_name, O_RDWR);
        if (ctx->send_queue == (mqd_t)-1) {
            perror("mq_open (открытие первой очереди)");
            return -1;
        }
        
        ctx->recv_queue = mq_open(ctx->recv_queue_name, O_RDWR);
        if (ctx->recv_queue == (mqd_t)-1) {
            perror("mq_open (открытие второй очереди)");
            mq_close(ctx->send_queue);
            ctx->send_queue = (mqd_t)-1;
            return -1;
        }
        
        printf("Открыты существующие очереди\n");
        printf("  Отправка через: %s\n", ctx->send_queue_name);
        printf("  Прием через: %s\n", ctx->recv_queue_name);
    }
    
    return 0;
}