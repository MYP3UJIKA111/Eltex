#include "permissions.h"

static FilePermissions perm = {0};

void displayCurrentPermissions(void) {
    if (perm.valid) {
        printf("\n--- Текущее состояние буфера ---\n");
        displayPermissions(&perm);
        if (strlen(perm.filepath) > 0 && strcmp(perm.filepath, "<Ручной ввод>") != 0) {
            displayComparison(perm.filepath);
        }
    } else {
        printf("Права не установлены! Сначала выполните пункт 1 или 2.\n");
    }
}

int processArguments(int argc, char** argv) {
    if (argc < 2) return 0;
    
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        printHelp();
        return 1;
    }
    
    if (argc == 2) {
        if (getPermissionsFromFile(argv[1], &perm) == 0) {
            printf("\nПрава файла '%s' успешно загружены в буфер!\n", argv[1]);
            displayPermissions(&perm);
            displayComparison(argv[1]);
            return 1;
        }
        return 0;
    }
    return 0;
}

int main(int argc, char** argv) {
    perm.valid = 0;
    perm.filepath[0] = '\0';
    
    printf("========================================\n");
    printf("  РАСЧЕТ МАСКИ ПРАВ ДОСТУПА\n");
    printf("========================================\n");

    
    if (argc > 1) {
        if (processArguments(argc, argv)) {
            printf("\nДля продолжения работы используйте меню.\n");
        }
    }
    
    while (1) {
        printf("\n========================================\n");
        printf("  МЕНЮ\n");
        printf("========================================\n");
        printf(" 1) Ввести права вручную (буквенные или цифровые)\n");
        printf(" 2) Получить права файла через stat()\n");
        printf(" 3) Изменить права (симуляция chmod в буфере)\n");
        printf(" 4) Показать текущие права и сравнение с ls -l\n");
        printf(" 5) Справка\n");
        printf(" 0) Выход\n");
        printf("========================================\n");
        printf("Ваш выбор: ");
        
        int choice;
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода! Введите число.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
        
        switch (choice) {
            case 1: {
                char input[BUFFER_SIZE];
                printf("\nВведите права (например, 755, 0644 или rwxr-x---): ");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                
                int numeric = -1;
                if (isValidNumeric(input)) {
                    numeric = parseNumericInput(input);
                } else if (isValidSymbolic(input)) {
                    numeric = parseSymbolicToNumeric(input);
                }
                
                if (numeric >= 0) {
                    // Если файл уже был загружен — сохраняем его имя для сравнения
                    // Иначе помечаем как ручной ввод
                    const char* target_name = (strlen(perm.filepath) > 0 && 
                                               strcmp(perm.filepath, "<Ручной ввод>") != 0) 
                                              ? perm.filepath 
                                              : "<Ручной ввод>";
                                              
                    savePermissionsToBuffer(&perm, numeric, target_name);
                    printf("Права успешно сохранены в буфер!\n");
                    displayPermissions(&perm);
                } else {
                    printf("Ошибка: Неверный формат прав!\n");
                }
                break;
            }
            
            case 2: {
                char filename[MAX_PATH];
                printf("\nВведите имя файла: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = '\0';
                
                if (strlen(filename) == 0) {
                    printf("Ошибка: Имя файла не может быть пустым!\n");
                    break;
                }
                
                if (access(filename, F_OK) != 0) {
                    printf("Ошибка: Файл '%s' не существует!\n", filename);
                    break;
                }
                
                if (getPermissionsFromFile(filename, &perm) == 0) {
                    printf("\nПрава файла успешно загружены в буфер!\n");
                    displayPermissions(&perm);
                    displayComparison(filename);
                }
                break;
            }
            
            case 3: {
                if (!perm.valid) {
                    printf("Ошибка: Сначала установите права (пункт 1 или 2)!\n");
                    break;
                }
                
                printf("\n========================================\n");
                printf("  СИМУЛЯЦИЯ ИЗМЕНЕНИЯ ПРАВ (В БУФЕРЕ)\n");
                printf("========================================\n");
                printf("Источник: %s\n", perm.filepath);
                printf("Текущие права: %s (0%o)\n", perm.symbolic, perm.numeric);
                printf("\nПримеры команд:\n");
                printf("  u+x   - добавить исполнение владельцу\n");
                printf("  g-w   - убрать запись у группы\n");
                printf("  o=r   - установить только чтение для других\n");
                printf("  a+x   - добавить исполнение для всех\n");
                printf("  755   - установить права 755\n");
                printf("  rwxr-x--- - установить полные буквенные права\n");
                printf("\nВНИМАНИЕ: Файл на диске НЕ будет изменен!\n");
                printf("Введите команду: ");
                
                char command[BUFFER_SIZE];
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = '\0';
                
                if (strlen(command) == 0) {
                    printf("Ошибка: Пустая команда!\n");
                    break;
                }
                
                FilePermissions backup = perm;
                
                if (modifyPermissionsInBuffer(&perm, command) == 0) {
                    if (perm.numeric != backup.numeric) {
                        printf("\n=== УСПЕШНО ИЗМЕНЕНО В БУФЕРЕ ===\n");
                        printf("Было : %s (0%o)\n", backup.symbolic, backup.numeric);
                        printf("Стало: %s (0%o)\n", perm.symbolic, perm.numeric);
                        printf("\n!!! Файл на диске НЕ ИЗМЕНИЛСЯ !!!\n");
                        displayPermissions(&perm);
                    } else {
                        printf("Права не изменились. Проверьте команду.\n");
                    }
                }
                break;
            }
            
            case 4: {
                displayCurrentPermissions();
                break;
            }
            
            case 5: {
                printHelp();
                break;
            }
            
            case 0: {
                printf("\nВыход из программы. До свидания!\n");
                if (perm.valid && strlen(perm.filepath) > 0 && 
                    strcmp(perm.filepath, "<Ручной ввод>") != 0) {
                    struct stat st;
                    if (stat(perm.filepath, &st) == 0) {
                        int current_mode = st.st_mode & 0777;
                        if (current_mode == perm.numeric) {
                            printf("Файл %s имеет такие же права (0%o).\n", 
                                   perm.filepath, current_mode);
                        } else {
                            printf("Файл %s имеет другие права: 0%o\n", 
                                   perm.filepath, current_mode);
                        }
                    }
                }
                return 0;
            }
            
            default: {
                printf("Неверный выбор! Введите число от 0 до 5.\n");
                break;
            }
        }
        
        printf("\nНажмите Enter для продолжения...");
        getchar();
    }
    
    return 0;
}