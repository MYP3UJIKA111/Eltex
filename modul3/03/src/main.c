#include "chat.h"

/* Глобальный контекст для обработчика сигнала */
static chat_context_t *global_ctx = NULL;

void chat_signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nПолучен сигнал SIGINT. Завершение работы...\n");
        if (global_ctx) {
            global_ctx->running = false;
            /* Отправляем сообщение о завершении */
            chat_send_message(global_ctx, "EXIT", MSG_PRIORITY_EXIT);
        }
    }
}

int main(int argc, char *argv[]) {
    chat_context_t ctx;
    char input_buffer[MAX_MSG_SIZE];
    
    if (argc != 2) {
        printf("Использование: %s <имя_очереди>\n", argv[0]);
        printf("Пример: %s my_chat\n", argv[0]);
        return 1;
    }
    
    /* Инициализация контекста */
    if (chat_init(&ctx, argv[1]) != 0) {
        fprintf(stderr, "Ошибка инициализации чата\n");
        return 1;
    }
    
    /* Устанавливаем глобальный контекст для обработчика сигнала */
    global_ctx = &ctx;
    
    /* Устанавливаем обработчик SIGINT */
    struct sigaction sa;
    sa.sa_handler = chat_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    
    /* Создаем или открываем очереди */
    if (chat_create_queues(&ctx) != 0) {
        fprintf(stderr, "Ошибка создания/открытия очередей\n");
        return 1;
    }
    
    /* Запускаем поток приема сообщений */
    if (pthread_create(&ctx.recv_thread, NULL, chat_receive_thread, &ctx) != 0) {
        perror("pthread_create");
        chat_cleanup(&ctx);
        return 1;
    }
    
    printf("\n=== P2P Чат запущен ===\n");
    printf("Введите сообщения для отправки (или 'exit' для выхода)\n");
    printf("Очередь отправки: %s\n", ctx.send_queue_name);
    printf("Очередь приема: %s\n", ctx.recv_queue_name);
    printf("========================\n\n");
    
    /* Основной цикл - чтение ввода пользователя */
    while (ctx.running) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            break;
        }
        
        /* Удаляем символ новой строки */
        input_buffer[strcspn(input_buffer, "\n")] = '\0';
        
        /* Проверка на команду выхода */
        if (strcmp(input_buffer, "exit") == 0) {
            printf("Завершение работы...\n");
            ctx.running = false;
            chat_send_message(&ctx, "EXIT", MSG_PRIORITY_EXIT);
            break;
        }
        
        /* Отправляем сообщение */
        if (chat_send_message(&ctx, input_buffer, MSG_PRIORITY_NORMAL) != 0) {
            fprintf(stderr, "Ошибка отправки сообщения\n");
        }
    }
    
    /* Ожидаем завершения потока приема */
    pthread_join(ctx.recv_thread, NULL);
    
    /* Очистка ресурсов */
    chat_cleanup(&ctx);
    
    printf("Чат завершен\n");
    return 0;
}