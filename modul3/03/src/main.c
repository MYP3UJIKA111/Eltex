#include "chat.h"
#include <sys/select.h>

static chat_context_t *global_ctx = NULL;

void chat_signal_handler(int sig) {
    
    if ((sig == SIGINT || sig == SIGTERM) && global_ctx != NULL) {
        global_ctx->running = 0;
        global_ctx->local_exit = 1;
    }
}

int main(int argc, char *argv[]) {
    chat_context_t ctx;

    if (argc != 2) {
        printf("Использование: %s <имя_очереди>\n", argv[0]);
        printf("Пример: %s my_chat\n", argv[0]);
        return 1;
    }

    if (chat_init(&ctx, argv[1]) != 0) {
        fprintf(stderr, "Ошибка инициализации чата\n");
        return 1;
    }

    global_ctx = &ctx;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = chat_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
        return 1;
    }

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM");
        return 1;
    }

    if (chat_create_queues(&ctx) != 0) {
        fprintf(stderr, "Ошибка создания/открытия очередей\n");
        return 1;
    }

    if (!ctx.running) {
        chat_cleanup(&ctx);
        return 0;
    }

    if (pthread_create(&ctx.recv_thread, NULL, chat_receive_thread, &ctx) != 0) {
        perror("pthread_create");
        chat_cleanup(&ctx);
        return 1;
    }

    printf("\n=== P2P Чат запущен ===\n");
    printf("Очередь отправки: %s\n", ctx.send_queue_name);
    printf("Очередь приема:   %s\n", ctx.recv_queue_name);

    chat_print_help();

    char input_buffer[MAX_MSG_SIZE + 1];

    while (ctx.running) {
        fd_set rfds;
        struct timeval timeout;

        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);

        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        int sel = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &timeout);

        if (sel == -1) {
            if (errno == EINTR) {
                continue;
            }

            perror("select");
            ctx.local_exit = 1;
            break;
        }

        if (sel == 0) {
            continue;
        }

        if (!FD_ISSET(STDIN_FILENO, &rfds)) {
            continue;
        }

        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            ctx.local_exit = 1;
            break;
        }

        input_buffer[strcspn(input_buffer, "\n")] = '\0';

        if (strcmp(input_buffer, "exit") == 0) {
            printf("Завершение работы...\n");
            ctx.local_exit = 1;
            break;
        }

        if (input_buffer[0] == '\0') {
            continue;
        }

        if (chat_send_message(&ctx, input_buffer, MSG_PRIORITY_NORMAL) != 0) {
            fprintf(stderr, "Ошибка отправки сообщения\n");
        }
    }

    if (ctx.local_exit) {
        chat_send_message(&ctx, "EXIT", MSG_PRIORITY_EXIT);
        usleep(200000);
    }

    ctx.running = 0;

    pthread_join(ctx.recv_thread, NULL);

    chat_cleanup(&ctx);

    printf("Чат завершен\n");

    return 0;
}