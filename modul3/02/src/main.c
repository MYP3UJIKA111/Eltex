#include "broker.h"

static broker_context_t *global_ctx = NULL;

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("\nПолучен сигнал завершения. Остановка...\n");
        if (global_ctx) {
            global_ctx->running = false;
        }
    }
}

void setup_signal_handlers(void) {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

void print_usage(const char *progname) {
    printf("Использование:\n");
    printf("  %s -b              - Запуск в режиме брокера\n", progname);
    printf("  %s -p <topic>      - Запуск в режиме издателя (тема: %s)\n", progname, "<topic>");
    printf("  %s -s <topic1> <topic2> ... - Запуск в режиме подписчика\n", progname);
    printf("\nПримеры:\n");
    printf("  %s -b\n", progname);
    printf("  %s -p news\n", progname);
    printf("  %s -s news weather\n", progname);
}

int main(int argc, char *argv[]) {
    key_t key;
    broker_context_t ctx;
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    /* Получаем ключ очереди */
    key = get_queue_key();
    if (key == -1) {
        fprintf(stderr, "Ошибка получения ключа очереди\n");
        return 1;
    }
    
    memset(&ctx, 0, sizeof(ctx));
    ctx.running = true;
    ctx.is_broker = false;
    global_ctx = &ctx;
    
    setup_signal_handlers();
    
    if (strcmp(argv[1], "-b") == 0) {
        /* Режим брокера */
        ctx.is_broker = true;
        if (broker_run(&ctx) != 0) {
            fprintf(stderr, "Ошибка запуска брокера\n");
            return 1;
        }
    } else if (strcmp(argv[1], "-p") == 0) {
        /* Режим издателя */
        if (argc < 3) {
            fprintf(stderr, "Ошибка: не указана тема\n");
            print_usage(argv[0]);
            return 1;
        }
        if (publisher_run(key, argv[2]) != 0) {
            fprintf(stderr, "Ошибка запуска издателя\n");
            return 1;
        }
    } else if (strcmp(argv[1], "-s") == 0) {
        /* Режим подписчика */
        if (argc < 3) {
            fprintf(stderr, "Ошибка: не указана тема для подписки\n");
            print_usage(argv[0]);
            return 1;
        }
        if (subscriber_run(key, &argv[2]) != 0) {
            fprintf(stderr, "Ошибка запуска подписчика\n");
            return 1;
        }
    } else {
        fprintf(stderr, "Неизвестный режим: %s\n", argv[1]);
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}