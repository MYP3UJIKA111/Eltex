#include "calculator.h"

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
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
        printf("Ошибка ввода! Введите неотрицательное число.\n");
        clearInputBuffer();
    }
}

void showDynamicMenu(Command commands[], int count) {
    printf("\n========================================\n");
    printf("   КАЛЬКУЛЯТОР С ДИНАМИЧЕСКОЙ ЗАГРУЗКОЙ\n");
    printf("========================================\n");
    
    if (count == 0) {
        printf("  (В каталоге plugins/ не найдено библиотек)\n");
    }
    
    for (int i = 0; i < count; i++) {
        if (commands[i].is_available) {
            printf(" %2d) %-20s (%s)\n", i + 1, 
                   commands[i].name, 
                   commands[i].symbol);
        }
    }
    
    printf("  0) Выход\n");
    printf("========================================\n");
    printf("Загружено функций: %d\n", count);
    printf("Выберите действие (0-%d): ", count);
}

void executeCommand(Command commands[], int count, int choice) {
    if (choice < 1 || choice > count) {
        printf("Ошибка: Неверный номер команды!\n");
        return;
    }
    
    Command* cmd = &commands[choice - 1];
    if (!cmd->is_available || cmd->func == NULL) {
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
            printf("Ошибка: Слишком много аргументов (максимум 10)!\n");
            free(args);
            return;
    }
    
    printf("Результат: %.2lf\n", result);
    free(args);
}

int loadPlugins(Command commands[], int* count) {
    DIR* dir = opendir(PLUGINS_DIR);
    if (dir == NULL) {
        printf("Ошибка: Не удалось открыть каталог '%s'\n", PLUGINS_DIR);
        printf("Создайте каталог и поместите туда .so файлы.\n");
        return 0;
    }
    
    printf("\n--- Сканирование каталога '%s' ---\n", PLUGINS_DIR);
    
    struct dirent* entry;
    int loaded = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        const char* ext = strstr(entry->d_name, ".so");
        if (ext == NULL || strcmp(ext, ".so") != 0) continue;
        
        char lib_path[MAX_PATH_LEN];
        snprintf(lib_path, sizeof(lib_path), "%s/%s", PLUGINS_DIR, entry->d_name);
        
        void* handle = dlopen(lib_path, RTLD_LAZY);
        if (handle == NULL) {
            printf("[!] Ошибка загрузки %s: %s\n", entry->d_name, dlerror());
            continue;
        }
        
        dlerror();
        
        // Стандартный POSIX-способ получения указателя на функцию из dlsym
        variadic_op_t func;
        *(void **)(&func) = dlsym(handle, "calculate");
        
        string_getter_t get_name;
        *(void **)(&get_name) = dlsym(handle, "get_name");
        
        string_getter_t get_symbol;
        *(void **)(&get_symbol) = dlsym(handle, "get_symbol");
        
        const char* err = dlerror();
        if (err != NULL) {
            printf("[!] В %s не найдена функция 'calculate': %s\n", entry->d_name, err);
            dlclose(handle);
            continue;
        }
        
        const char* name = get_name ? get_name() : entry->d_name;
        const char* symbol = get_symbol ? get_symbol() : "?";
        
        if (*count >= MAX_COMMANDS) {
            printf("[!] Превышен лимит команд (%d)\n", MAX_COMMANDS);
            dlclose(handle);
            break;
        }
        
        Command* cmd = &commands[*count];
        strncpy(cmd->name, name, MAX_CMD_NAME - 1);
        strncpy(cmd->symbol, symbol, MAX_CMD_NAME - 1);
        strncpy(cmd->lib_path, lib_path, MAX_PATH_LEN - 1);
        cmd->func = func;
        cmd->handle = handle;
        cmd->is_available = 1;
        
        printf("[✓] Загружено: %-20s (%s) из %s\n", name, symbol, entry->d_name);
        
        (*count)++;
        loaded++;
    }
    
    closedir(dir);
    printf("--- Загружено плагинов: %d ---\n\n", loaded);
    
    return loaded;
}

void unloadPlugins(Command commands[], int count) {
    for (int i = 0; i < count; i++) {
        if (commands[i].handle != NULL) {
            dlclose(commands[i].handle);
            commands[i].handle = NULL;
        }
    }
}

int main(void) {
    Command commands[MAX_COMMANDS];
    int command_count = 0;
    int choice;
    
    printf("========================================\n");
    printf("  КАЛЬКУЛЯТОР С ПЛАГИНАМИ              \n");
    printf("  (динамическая загрузка функций)      \n");
    printf("========================================\n");
    
    int loaded = loadPlugins(commands, &command_count);
    
    if (loaded == 0) {
        printf("\nВнимание: не найдено ни одного плагина!\n");
        printf("Создайте каталог 'plugins/' и поместите туда .so файлы.\n");
    }
    
    while (1) {
        showDynamicMenu(commands, command_count);
        
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода! Введите число.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
        
        if (choice == 0) {
            printf("\nВыход из программы...\n");
            printf("Выгрузка плагинов...\n");
            unloadPlugins(commands, command_count);
            printf("До свидания!\n");
            break;
        }
        
        if (choice >= 1 && choice <= command_count) {
            executeCommand(commands, command_count, choice);
        } else {
            printf("Ошибка: Неверный номер команды!\n");
        }
        
        printf("\nНажмите Enter для продолжения...");
        getchar();
    }
    
    return 0;
}