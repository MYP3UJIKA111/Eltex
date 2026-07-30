#include "priority_queue.h"
#include <locale.h>

void generateRandomMessage(char* buffer, int size) {
    const char* messages[] = {
        "Системное уведомление",
        "Ошибка подключения",
        "Важное сообщение",
        "Обычное событие",
        "Критическая ошибка",
        "Предупреждение",
        "Информационное сообщение",
        "Требует внимания",
        "Фоновая задача",
        "Срочное оповещение"
    };
    int index = rand() % 10;
    strncpy(buffer, messages[index], size - 1);
    buffer[size - 1] = '\0';
}

int main(void) {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    srand((unsigned int)time(NULL));
    
    QueueItem* queue = NULL;
    int choice;
    
    while (1) {
        printf("\n========================================\n");
        printf("  ОЧЕРЕДЬ С ПРИОРИТЕТОМ\n");
        printf("========================================\n");
        printf(" 1) Добавить сообщение (случайный приоритет)\n");
        printf(" 2) Добавить сообщение (указать приоритет)\n");
        printf(" 3) Извлечь первое сообщение (наивысший приоритет)\n");
        printf(" 4) Извлечь сообщение с указанным приоритетом\n");
        printf(" 5) Извлечь сообщение с приоритетом не ниже заданного\n");
        printf(" 6) Показать очередь\n");
        printf(" 7) Сгенерировать N сообщений\n");
        printf(" 8) Очистить очередь\n");
        printf(" 0) Выход\n");
        printf("========================================\n");
        printf("Ваш выбор: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода!\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');
        
        switch (choice) {
            case 1: {
                int priority = rand() % 256;
                char message[MAX_MSG_LEN];
                generateRandomMessage(message, MAX_MSG_LEN);
                
                queue = enqueue(queue, priority, message);
                printf("Добавлено: [Приоритет: %d] %s\n", priority, message);
                break;
            }
            
            case 2: {
                int priority;
                char message[MAX_MSG_LEN];
                
                printf("Введите приоритет (0-255): ");
                if (scanf("%d", &priority) != 1 || priority < 0 || priority > 255) {
                    printf("Ошибка: Приоритет должен быть от 0 до 255!\n");
                    while (getchar() != '\n');
                    break;
                }
                while (getchar() != '\n');
                
                printf("Введите сообщение: ");
                if (fgets(message, sizeof(message), stdin) == NULL) break;
                message[strcspn(message, "\n")] = '\0';
                
                queue = enqueue(queue, priority, message);
                printf("Добавлено: [Приоритет: %d] %s\n", priority, message);
                break;
            }
            
            case 3: {
                QueueItem* item = dequeueFirst(&queue);
                if (item == NULL) {
                    printf("Очередь пуста!\n");
                } else {
                    printf("Извлечено: [Приоритет: %d] %s\n", item->priority, item->message);
                    free(item);
                }
                break;
            }
            
            case 4: {
                int priority;
                printf("Введите приоритет для извлечения (0-255): ");
                if (scanf("%d", &priority) != 1) {
                    printf("Ошибка ввода!\n");
                    while (getchar() != '\n');
                    break;
                }
                while (getchar() != '\n');
                
                QueueItem* item = dequeueByPriority(&queue, priority);
                if (item == NULL) {
                    printf("Сообщений с приоритетом %d не найдено!\n", priority);
                } else {
                    printf("Извлечено: [Приоритет: %d] %s\n", item->priority, item->message);
                    free(item);
                }
                break;
            }
            
            case 5: {
                int minPriority;
                printf("Введите минимальный приоритет (0-255): ");
                if (scanf("%d", &minPriority) != 1) {
                    printf("Ошибка ввода!\n");
                    while (getchar() != '\n');
                    break;
                }
                while (getchar() != '\n');
                
                QueueItem* item = dequeueByMinPriority(&queue, minPriority);
                if (item == NULL) {
                    printf("Сообщений с приоритетом >= %d не найдено!\n", minPriority);
                } else {
                    printf("Извлечено: [Приоритет: %d] %s\n", item->priority, item->message);
                    free(item);
                }
                break;
            }
            
            case 6: {
                printQueue(queue);
                break;
            }
            
            case 7: {
                int n;
                printf("Сколько сообщений сгенерировать? ");
                if (scanf("%d", &n) != 1 || n <= 0) {
                    printf("Ошибка: Введите положительное число!\n");
                    while (getchar() != '\n');
                    break;
                }
                while (getchar() != '\n');
                
                for (int i = 0; i < n; i++) {
                    int priority = rand() % 256;
                    char message[MAX_MSG_LEN];
                    generateRandomMessage(message, MAX_MSG_LEN);
                    queue = enqueue(queue, priority, message);
                }
                printf("Сгенерировано %d сообщений.\n", n);
                break;
            }
            
            case 8: {
                freeQueue(queue);
                queue = NULL;
                printf("Очередь очищена.\n");
                break;
            }
            
            case 0: {
                freeQueue(queue);
                printf("Выход из программы. До свидания!\n");
                return 0;
            }
            
            default: {
                printf("Неверный выбор!\n");
                break;
            }
        }
    }
    
    return 0;
}