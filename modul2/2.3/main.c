#include "calculator.h"

void setupCommands(Command commands[], int* count) {
    initCommands(commands, count);
    
    addCommand(commands, count, "Сумма", "Σ", sum);
    addCommand(commands, count, "Произведение", "Π", multiply_variadic);
    addCommand(commands, count, "Максимум", "max", max_value);
    addCommand(commands, count, "Минимум", "min", min_value);
    addCommand(commands, count, "Среднее", "avg", average);
}

void addCustomCommand(Command commands[], int* count) {
    char name[MAX_CMD_NAME], symbol[MAX_CMD_NAME];
    
    printf("\n--- Добавление пользовательской команды ---\n");
    printf("Название команды: ");
    fgets(name, MAX_CMD_NAME, stdin);
    name[strcspn(name, "\n")] = '\0';
    
    printf("Символ операции: ");
    fgets(symbol, MAX_CMD_NAME, stdin);
    symbol[strcspn(symbol, "\n")] = '\0';
    
    int func_type;
    printf("Выберите тип функции:\n");
    printf("1 - Сумма\n");
    printf("2 - Произведение\n");
    printf("3 - Максимум\n");
    printf("4 - Минимум\n");
    printf("5 - Среднее\n");
    printf("Ваш выбор: ");
    
    if (scanf("%d", &func_type) != 1) {
        clearInputBuffer();
        printf("Ошибка ввода!\n");
        return;
    }
    clearInputBuffer();
    
    variadic_op_t selected_func = NULL;
    switch (func_type) {
        case 1: selected_func = sum; break;
        case 2: selected_func = multiply_variadic; break;
        case 3: selected_func = max_value; break;
        case 4: selected_func = min_value; break;
        case 5: selected_func = average; break;
        default:
            printf("Неверный выбор!\n");
            return;
    }
    
    addCommand(commands, count, name, symbol, selected_func);
    printf("Команда '%s' успешно добавлена!\n", name);
}

int main(void) {
    Command commands[MAX_COMMANDS];
    int command_count = 0;
    int choice;
    
    printf("========================================\n");
    printf("  ДОБРО ПОЖАЛОВАТЬ В КАЛЬКУЛЯТОР!      \n");
    printf("  (Функции с переменным числом          \n");
    printf("   параметров)                          \n");
    printf("========================================\n");
    
    setupCommands(commands, &command_count);
    
    while (1) {
        showDynamicMenu(commands, command_count);
        
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода! Введите число.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
        
        if (choice == 0) {
            printf("\nВыход из программы. До свидания!\n");
            break;
        }
        
        if (choice == -1) {
            addCustomCommand(commands, &command_count);
            continue;
        }
        
        if (choice >= 1 && choice <= command_count) {
            executeCommand(commands, command_count, choice);
        } else {
            printf("Ошибка: Неверный номер команды!\n");
            printf("Для добавления команды введите -1\n");
        }
        
        printf("\nНажмите Enter для продолжения...");
        getchar();
    }
    
    return 0;
}