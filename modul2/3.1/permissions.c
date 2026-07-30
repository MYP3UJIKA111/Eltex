#include "permissions.h"

void parseNumericToSymbolic(int numeric, char* symbolic) {
    int owner = (numeric >> 6) & 7;
    int group = (numeric >> 3) & 7;
    int other = numeric & 7;
    
    const char* perms[] = {"---", "--x", "-w-", "-wx", 
                           "r--", "r-x", "rw-", "rwx"};
    
    sprintf(symbolic, "%s%s%s", perms[owner], perms[group], perms[other]);
}

int parseSymbolicToNumeric(const char* symbolic) {
    if (strlen(symbolic) != 9 && strlen(symbolic) != 10) {
        return -1;
    }
    
    int pos = (strlen(symbolic) == 10) ? 1 : 0;
    int owner = 0, group = 0, other = 0;
    
    for (int i = 0; i < 3; i++) {
        int value = 0;
        char r = symbolic[pos + i * 3];
        char w = symbolic[pos + i * 3 + 1];
        char x = symbolic[pos + i * 3 + 2];
        
        if (r == 'r' || r == 'R') value |= 4;
        if (w == 'w' || w == 'W') value |= 2;
        if (x == 'x' || x == 'X' || x == 's' || x == 'S' || 
            x == 't' || x == 'T') value |= 1;
        
        if (i == 0) owner = value;
        else if (i == 1) group = value;
        else other = value;
    }
    
    return owner * 64 + group * 8 + other;
}

int parseNumericInput(const char* str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] < '0' || str[i] > '7') return -1;
    }
    
    int result = 0;
    for (int i = 0; i < len; i++) {
        result = result * 8 + (str[i] - '0');
    }
    
    if (result < 0 || result > 07777) return -1;
    return result;
}

// Вывод ровно 12 бит: 3 бита спец. прав (SUID/SGID/Sticky) + 9 бит основных прав
void printBinary(unsigned short value, char* buffer, int size) {
    int pos = 0;
    for (int i = 11; i >= 0; i--) {
        if (i == 8 || i == 5 || i == 2) {
            if (pos < size - 1) buffer[pos++] = ' ';
        }
        buffer[pos++] = (value & (1 << i)) ? '1' : '0';
    }
    buffer[pos] = '\0';
}

void savePermissionsToBuffer(FilePermissions* perm, int mode, const char* filename) {
    if (filename != NULL && strlen(filename) > 0) {
        strncpy(perm->filepath, filename, MAX_PATH - 1);
        perm->filepath[MAX_PATH - 1] = '\0';
    } else {
        perm->filepath[0] = '\0';
    }
    perm->numeric = mode & 07777;
    perm->binary = mode & 07777;
    perm->valid = 1;
    parseNumericToSymbolic(mode & 0777, perm->symbolic);
}

int getPermissionsFromFile(const char* filename, FilePermissions* perm) {
    struct stat file_stat;
    if (stat(filename, &file_stat) != 0) {
        fprintf(stderr, "Ошибка: Не удалось получить информацию о файле '%s'\n", filename);
        fprintf(stderr, "Причина: %s\n", strerror(errno));
        perm->valid = 0;
        return -1;
    }
    
    int mode = file_stat.st_mode & 07777;
    perm->st_mode = file_stat.st_mode;
    perm->st_uid = file_stat.st_uid;
    perm->st_gid = file_stat.st_gid;
    perm->st_size = file_stat.st_size;
    perm->st_mtime_val = file_stat.st_mtime;
    
    struct passwd *pw = getpwuid(file_stat.st_uid);
    if (pw != NULL) {
        strncpy(perm->owner, pw->pw_name, sizeof(perm->owner) - 1);
        perm->owner[sizeof(perm->owner) - 1] = '\0';
    } else {
        snprintf(perm->owner, sizeof(perm->owner), "%d", file_stat.st_uid);
    }
    
    struct group *gr = getgrgid(file_stat.st_gid);
    if (gr != NULL) {
        strncpy(perm->group, gr->gr_name, sizeof(perm->group) - 1);
        perm->group[sizeof(perm->group) - 1] = '\0';
    } else {
        snprintf(perm->group, sizeof(perm->group), "%d", file_stat.st_gid);
    }
    
    savePermissionsToBuffer(perm, mode, filename);
    return 0;
}

void displayFileInfo(const FilePermissions* perm) {
    if (!perm->valid) return;
    
    // Если это ручной ввод — не показываем метаданные файла
    if (strcmp(perm->filepath, "<Ручной ввод>") == 0) return;
    
    char time_str[64];
    struct tm *tm_info = localtime(&perm->st_mtime_val);
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", tm_info);
    
    printf("\nИнформация о файле:\n");
    printf("  Владелец: %s\n", perm->owner);
    printf("  Группа:   %s\n", perm->group);
    printf("  Размер:   %ld байт\n", (long)perm->st_size);
    printf("  Изменён:  %s\n", time_str);
}

void displayPermissions(const FilePermissions* perm) {
    if (!perm->valid) {
        printf("Нет данных о файле\n");
        return;
    }
    
    char binary_str[256];
    printBinary(perm->binary, binary_str, sizeof(binary_str));
    
    printf("\n========================================\n");
    if (strlen(perm->filepath) > 0) {
        printf("Файл / Источник : %s\n", perm->filepath);
    }
    printf("========================================\n");
    printf("Буквенное представление: %s\n", perm->symbolic);
    printf("Цифровое представление : %d (0%o)\n", perm->numeric, perm->numeric);
    printf("Битовое представление  : %s\n", binary_str);
    printf("========================================\n");
    
    displayFileInfo(perm);
}

void displayComparison(const char* filename) {
    if (strcmp(filename, "<Ручной ввод>") == 0) return;
    
    printf("\n--- Сравнение с системным выводом ---\n");
    char command[BUFFER_SIZE];
    
    snprintf(command, sizeof(command), "ls -l %s 2>/dev/null", filename);
    printf("$ %s\n", command);
    system(command);
    
    snprintf(command, sizeof(command), "stat %s 2>/dev/null | grep -E 'Access: \\('", filename);
    printf("$ %s\n", command);
    system(command);
    printf("---------------------------------------\n");
}

void printHelp(void) {
    printf("\n========================================\n");
    printf("  ИНСТРУКЦИЯ ПО ИСПОЛЬЗОВАНИЮ\n");
    printf("========================================\n");
    printf("Пункт 1: Ввод прав вручную (буквенные rwxr-x--- или цифровые 755)\n");
    printf("         -> показывается битовое представление\n");
    printf("Пункт 2: Чтение прав реального файла через функцию stat()\n");
    printf("         -> показывается буквенное, цифровое, битовое + сравнение с ls -l\n");
    printf("Пункт 3: Изменение прав в памяти (симуляция chmod)\n");
    printf("         -> показывается буквенное, цифровое, битовое\n");
    printf("         -> изменения НЕ применяются к файлу на диске!\n");
    printf("         Поддерживаемые форматы:\n");
    printf("         - Цифровые: 755, 0644\n");
    printf("         - Буквенные полные: rwxr-xr-x\n");
    printf("         - Модификация: u+x, g-w, o=r, a+x\n");
    printf("Пункт 4: Показать текущее состояние буфера\n");
    printf("========================================\n");
}

// Изменяет права только в структуре perm, не трогая файловую систему
int modifyPermissionsInBuffer(FilePermissions* perm, const char* command) {
    if (!perm->valid) {
        fprintf(stderr, "Ошибка: Сначала установите права (пункт 1 или 2)!\n");
        return -1;
    }
    
    int new_numeric = perm->numeric;
    char cmd_copy[BUFFER_SIZE];
    strncpy(cmd_copy, command, BUFFER_SIZE - 1);
    cmd_copy[BUFFER_SIZE - 1] = '\0';
    
    char* trimmed = cmd_copy;
    while (*trimmed == ' ') trimmed++;
    
    int len = strlen(trimmed);
    while (len > 0 && (trimmed[len-1] == ' ' || trimmed[len-1] == '\n')) {
        trimmed[len-1] = '\0';
        len--;
    }
    
    if (len == 0) {
        fprintf(stderr, "Ошибка: Пустая команда!\n");
        return -1;
    }
    
    // 1. Проверка на чисто числовой формат
    int numeric_val = parseNumericInput(trimmed);
    if (numeric_val >= 0) {
        savePermissionsToBuffer(perm, numeric_val, perm->filepath);
        return 0;
    }
    
    // 2. Проверка на полный буквенный формат
    if (isValidSymbolic(trimmed)) {
        int sym_numeric = parseSymbolicToNumeric(trimmed);
        if (sym_numeric >= 0) {
            savePermissionsToBuffer(perm, sym_numeric, perm->filepath);
            return 0;
        }
    }
    
    // 3. Парсинг команды модификации
    if (len >= 3) {
        int who_u = 0, who_g = 0, who_o = 0;
        char op = '\0';
        int perm_r = 0, perm_w = 0, perm_x = 0;
        
        int i = 0;
        while (i < len && trimmed[i] != '+' && trimmed[i] != '-' && trimmed[i] != '=') {
            if (trimmed[i] == 'u') who_u = 1;
            else if (trimmed[i] == 'g') who_g = 1;
            else if (trimmed[i] == 'o') who_o = 1;
            else if (trimmed[i] == 'a') { who_u = 1; who_g = 1; who_o = 1; }
            i++;
        }
        
        if (!who_u && !who_g && !who_o) { // По умолчанию 'a' (все)
            who_u = 1; who_g = 1; who_o = 1;
        }
        
        if (i < len && (trimmed[i] == '+' || trimmed[i] == '-' || trimmed[i] == '=')) {
            op = trimmed[i];
            i++;
        } else {
            fprintf(stderr, "Ошибка: Неверный формат команды (нет оператора +, - или =)\n");
            return -1;
        }
        
        while (i < len) {
            if (trimmed[i] == 'r' || trimmed[i] == 'R') perm_r = 1;
            else if (trimmed[i] == 'w' || trimmed[i] == 'W') perm_w = 1;
            else if (trimmed[i] == 'x' || trimmed[i] == 'X') perm_x = 1;
            i++;
        }
        
        int rights_value = (perm_r ? 4 : 0) | (perm_w ? 2 : 0) | (perm_x ? 1 : 0);
        
        int owner = (new_numeric >> 6) & 7;
        int group = (new_numeric >> 3) & 7;
        int other = new_numeric & 7;
        
        if (op == '+') {
            if (who_u) owner |= rights_value;
            if (who_g) group |= rights_value;
            if (who_o) other |= rights_value;
        } else if (op == '-') {
            if (who_u) owner &= ~rights_value;
            if (who_g) group &= ~rights_value;
            if (who_o) other &= ~rights_value;
        } else if (op == '=') {
            if (who_u) owner = rights_value;
            if (who_g) group = rights_value;
            if (who_o) other = rights_value;
        }
        
        new_numeric = (owner << 6) | (group << 3) | other;
        savePermissionsToBuffer(perm, new_numeric, perm->filepath);
        return 0;
    }
    
    fprintf(stderr, "Ошибка: Неверный формат команды!\n");
    fprintf(stderr, "Примеры: 755, u+x, g-w, o=r, a+x, rwxr-x---\n");
    return -1;
}

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int isValidSymbolic(const char* str) {
    if (strlen(str) != 9 && strlen(str) != 10) return 0;
    int pos = (strlen(str) == 10) ? 1 : 0;
    for (int i = 0; i < 9; i++) {
        char c = str[pos + i];
        if (i % 3 == 0 && c != 'r' && c != 'R' && c != '-') return 0;
        if (i % 3 == 1 && c != 'w' && c != 'W' && c != '-') return 0;
        if (i % 3 == 2 && c != 'x' && c != 'X' && c != 's' && c != 'S' && 
            c != 't' && c != 'T' && c != '-') return 0;
    }
    return 1;
}

int isValidNumeric(const char* str) {
    int len = strlen(str);
    if (len < 1 || len > 4) return 0;
    for (int i = 0; i < len; i++) {
        if (str[i] < '0' || str[i] > '7') return 0;
    }
    return 1;
}