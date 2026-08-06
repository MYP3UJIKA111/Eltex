#include "chat.h"

int chat_init(chat_context_t *ctx, const char *base_name) {
    if (!ctx || !base_name || base_name[0] == '\0') {
        return -1;
    }

    memset(ctx, 0, sizeof(chat_context_t));

    ctx->send_queue = (mqd_t)-1;
    ctx->recv_queue = (mqd_t)-1;

    ctx->is_creator = false;
    ctx->running = 1;
    ctx->local_exit = 0;

    int rc;

    if (base_name[0] == '/') {
        rc = snprintf(ctx->queue_name,
                      sizeof(ctx->queue_name),
                      "%s",
                      base_name);
    } else {
        rc = snprintf(ctx->queue_name,
                      sizeof(ctx->queue_name),
                      "/%s",
                      base_name);
    }

    if (rc < 0 || (size_t)rc >= sizeof(ctx->queue_name)) {
        fprintf(stderr, "Имя очереди слишком длинное\n");
        return -1;
    }

    for (char *p = ctx->queue_name + 1; *p != '\0'; ++p) {
        if (*p == '/') {
            *p = '_';
        }
    }

    if (ctx->queue_name[1] == '\0') {
        fprintf(stderr, "Пустое имя очереди\n");
        return -1;
    }

    return 0;
}

int chat_create_queues(chat_context_t *ctx) {
    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));

    attr.mq_maxmsg = MAX_QUEUE_MSGS;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_flags = 0;

    char q1_name[QUEUE_NAME_MAX];
    char q2_name[QUEUE_NAME_MAX];

    int rc;

    rc = snprintf(q1_name, sizeof(q1_name), "%s_1", ctx->queue_name);
    if (rc < 0 || (size_t)rc >= sizeof(q1_name)) {
        fprintf(stderr, "Слишком длинное имя очереди\n");
        return -1;
    }

    rc = snprintf(q2_name, sizeof(q2_name), "%s_2", ctx->queue_name);
    if (rc < 0 || (size_t)rc >= sizeof(q2_name)) {
        fprintf(stderr, "Слишком длинное имя очереди\n");
        return -1;
    }


    mqd_t q1 = mq_open(q1_name,
                       O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK,
                       0666,
                       &attr);

    if (q1 != (mqd_t)-1) {
        ctx->is_creator = true;


        mqd_t q2 = mq_open(q2_name,
                           O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK,
                           0666,
                           &attr);

        if (q2 == (mqd_t)-1) {
            int saved_errno = errno;

            mq_close(q1);
            mq_unlink(q1_name);

            errno = saved_errno;
            perror("mq_open (создание второй очереди)");
            return -1;
        }

        ctx->recv_queue = q1;
        ctx->send_queue = q2;

        snprintf(ctx->recv_queue_name,
                 sizeof(ctx->recv_queue_name),
                 "%s",
                 q1_name);

        snprintf(ctx->send_queue_name,
                 sizeof(ctx->send_queue_name),
                 "%s",
                 q2_name);

        printf("Созданы новые очереди (создатель)\n");
        printf("  Прием через:   %s\n", ctx->recv_queue_name);
        printf("  Отправка через: %s\n", ctx->send_queue_name);

        return 0;
    }

    if (errno != EEXIST) {
        perror("mq_open (первая очередь)");
        return -1;
    }


    ctx->is_creator = false;

    q1 = mq_open(q1_name, O_RDWR | O_NONBLOCK);
    if (q1 == (mqd_t)-1) {
        perror("mq_open (открытие первой очереди)");
        return -1;
    }

    mqd_t q2 = (mqd_t)-1;

    for (int attempt = 0; attempt < 20; ++attempt) {
        q2 = mq_open(q2_name, O_RDWR | O_NONBLOCK);

        if (q2 != (mqd_t)-1) {
            break;
        }

        if (errno != ENOENT) {
            perror("mq_open (открытие второй очереди)");
            mq_close(q1);
            return -1;
        }

        usleep(100000);
    }

    if (q2 == (mqd_t)-1) {
        fprintf(stderr, "Вторая очередь %s не найдена\n", q2_name);
        mq_close(q1);
        return -1;
    }

   
    ctx->send_queue = q1;
    ctx->recv_queue = q2;

    snprintf(ctx->send_queue_name,
             sizeof(ctx->send_queue_name),
             "%s",
             q1_name);

    snprintf(ctx->recv_queue_name,
             sizeof(ctx->recv_queue_name),
             "%s",
             q2_name);

    printf("Открыты существующие очереди\n");
    printf("  Отправка через: %s\n", ctx->send_queue_name);
    printf("  Прием через:   %s\n", ctx->recv_queue_name);

    return 0;
}