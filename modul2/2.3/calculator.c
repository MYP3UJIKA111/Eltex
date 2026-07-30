#include "calculator.h"

double sum(int num_args, ...) {
    va_list args;
    va_start(args, num_args);
    double result = 0.0;
    for (int i = 0; i < num_args; i++) {
        result += va_arg(args, double);
    }
    va_end(args);
    return result;
}

double multiply_variadic(int num_args, ...) {
    va_list args;
    va_start(args, num_args);
    double result = 1.0;
    for (int i = 0; i < num_args; i++) {
        result *= va_arg(args, double);
    }
    va_end(args);
    return result;
}

double max_value(int num_args, ...) {
    va_list args;
    va_start(args, num_args);
    double max = va_arg(args, double);
    for (int i = 1; i < num_args; i++) {
        double current = va_arg(args, double);
        if (current > max) max = current;
    }
    va_end(args);
    return max;
}

double min_value(int num_args, ...) {
    va_list args;
    va_start(args, num_args);
    double min = va_arg(args, double);
    for (int i = 1; i < num_args; i++) {
        double current = va_arg(args, double);
        if (current < min) min = current;
    }
    va_end(args);
    return min;
}

double average(int num_args, ...) {
    if (num_args == 0) {
        printf("Ошибка: Нет аргументов для усреднения!\n");
        return 0;
    }
    va_list args;
    va_start(args, num_args);
    double sum = 0.0;
    for (int i = 0; i < num_args; i++) {
        sum += va_arg(args, double);
    }
    va_end(args);
    return sum / num_args;
}

void initCommands(Command commands[], int* count) {
    *count = 0;
    memset(commands, 0, sizeof(Command) * MAX_COMMANDS);
}

void addCommand(Command commands[], int* count, const char* name, 
                const char* symbol, variadic_op_t func) {
    if (*count >= MAX_COMMANDS) {
        printf("Ошибка: Превышено максимальное количество команд!\n");
        return;
    }
    strncpy(commands[*count].name, name, MAX_CMD_NAME - 1);
    strncpy(commands[*count].symbol, symbol, MAX_CMD_NAME - 1);
    commands[*count].func = func;
    commands[*count].is_available = 1;
    (*count)++;
}

void showDynamicMenu(Command commands[], int count) {
    printf("\n========================================\n");
    printf("         МАТЕМАТИЧЕСКИЙ КАЛЬКУЛЯТОР    \n");
    printf("         (функции с переменным числом   \n");
    printf("          параметров)                   \n");
    printf("========================================\n");
    
    for (int i = 0; i < count; i++) {
        if (commands[i].is_available) {
            printf(" %2d) %-20s (%s)\n", i + 1, 
                   commands[i].name, 
                   commands[i].symbol);
        }
    }
    
    printf("  0) Выход                             \n");
    printf("========================================\n");
    printf("Выберите действие (0-%d): ", count);
}

double getNumber(const char* prompt) {
    double num;
    while (1) {
        printf("%s", prompt);
        if (scanf("%lf", &num) == 1) {
            clearInputBuffer();
            return num;
        }
        printf("Ошибка ввода! Введите число.\n");
        clearInputBuffer();
    }
}

int getArgumentCount(void) {
    int n;
    while (1) {
        printf("Введите количество чисел: ");
        if (scanf("%d", &n) == 1 && n >= 0) {
            clearInputBuffer();
            return n;
        }
        printf("Ошибка ввода! Введите положительное число.\n");
        clearInputBuffer();
    }
}

void executeCommand(Command commands[], int count, int choice) {
    if (choice < 1 || choice > count) {
        printf("Ошибка: Неверный номер команды!\n");
        return;
    }
    
    int idx = choice - 1;
    Command* cmd = &commands[idx];
    
    if (!cmd->is_available) {
        printf("Ошибка: Команда недоступна!\n");
        return;
    }
    
    int num_args = getArgumentCount();
    if (num_args == 0) {
        printf("Результат: 0.00\n");
        return;
    }
    
    double* args = malloc(num_args * sizeof(double));
    if (!args) {
        printf("Ошибка выделения памяти!\n");
        return;
    }
    
    for (int i = 0; i < num_args; i++) {
        char prompt[50];
        snprintf(prompt, sizeof(prompt), "Введите число #%d: ", i + 1);
        args[i] = getNumber(prompt);
    }
    
    double result;
    switch (num_args) {                   
        case 0: result = cmd->func(0); break;
        case 1: result = cmd->func(1, args[0]); break;
        case 2: result = cmd->func(2, args[0], args[1]); break;
        case 3: result = cmd->func(3, args[0], args[1], args[2]); break;
        case 4: result = cmd->func(4, args[0], args[1], args[2], args[3]); break;
        case 5: result = cmd->func(5, args[0], args[1], args[2], args[3], args[4]); break;
        case 6: result = cmd->func(6, args[0], args[1], args[2], args[3], args[4], args[5]); break;
        case 7: result = cmd->func(7, args[0], args[1], args[2], args[3], args[4], args[5], args[6]); break;
        case 8: result = cmd->func(8, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]); break;
        case 9: result = cmd->func(9, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]); break;
        case 10: result = cmd->func(10, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]); break;
        default:
            printf("Ошибка: Слишком много аргументов!\n");
            free(args);
            return;
    }
    
    printf("Результат: %.2lf\n", result);
    free(args);
}

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}