#include "broker.h"

static broker_context_t *g_broker_ctx = NULL;

static void broker_sighandler(int sig) {
    (void)sig;
    if (g_broker_ctx) g_broker_ctx->running = 0;
}

static void print_usage(const char *progname) {
    printf("Использование:\n");
    printf("  %s -b                             - брокер\n", progname);
    printf("  %s -p <topic>                     - издатель\n", progname);
    printf("  %s -s <topic1> [topic2] [...]     - подписчик\n", progname);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    key_t key = get_queue_key();

    if (strcmp(argv[1], "-b") == 0) {
        broker_context_t ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.key = key;
        ctx.running = 1;
        ctx.is_broker = true;
        g_broker_ctx = &ctx;

        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = broker_sighandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);

        return broker_run(&ctx);
    } else if (strcmp(argv[1], "-p") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Укажите тему\n");
            return 1;
        }
        return publisher_run(key, argv[2]);
    } else if (strcmp(argv[1], "-s") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Укажите темы\n");
            return 1;
        }
        return subscriber_run(key, &argv[2], argc - 2);
    }

    print_usage(argv[0]);
    return 1;
}