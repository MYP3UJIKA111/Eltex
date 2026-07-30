#include "spravochnik.h"
#include <strings.h>

struct abonent database[MAX_ABONENTS];
int count = 0;

void removeNewline(char* str) {
    char* newline = strchr(str, '\n');
    if (newline) *newline = '\0';
}

void clearEntry(struct abonent* ab) {
    memset(ab, 0, sizeof(struct abonent));
}

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int strcmpIgnoreCase(const char* s1, const char* s2) {
    return strcasecmp(s1, s2);
}

int isOnlySpaces(const char* str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) return 0;
        str++;
    }
    return 1;
}

int isValidName(const char* name) {
    if (name[0] == '\0' || isOnlySpaces(name)) return 0;
    while (*name) {
        unsigned char c = (unsigned char)*name;
        if (c > 127) { name++; continue; }
        if (!isalpha(c) && !isspace(c)) return 0;
        name++;
    }
    return 1;
}

int isValidPhone(const char* phone) {
    if (phone[0] == '\0' || isOnlySpaces(phone)) return 0;
    int hasDigit = 0;
    while (*phone) {
        char c = *phone;
        if (isdigit((unsigned char)c)) hasDigit = 1;
        else if (c != '+' && c != '-' && c != ' ' && c != '(' && c != ')') return 0;
        phone++;
    }
    return hasDigit;
}

void printAbonent(const struct abonent* const ab, int index) {
    printf("Запись #%d:\n", index);
    printf("  ФИО: %s %s %s\n", ab->second_name, ab->first_name, ab->patronymic);
    printf("  Работа: %s, Должность: %s\n", ab->workplace, ab->position);
    for (int i = 0; i < MAX_PHONES; i++) {
        if (ab->phones[i][0] != '\0') printf("  Телефон: %s\n", ab->phones[i]);
    }
    for (int i = 0; i < MAX_EMAILS; i++) {
        if (ab->emails[i][0] != '\0') printf("  Email: %s\n", ab->emails[i]);
    }
    for (int i = 0; i < MAX_SOCIALS; i++) {
        if (ab->socials[i][0] != '\0') printf("  Соцсети: %s\n", ab->socials[i]);
    }
    for (int i = 0; i < MAX_MESSENGERS; i++) {
        if (ab->messengers[i][0] != '\0') printf("  Мессенджер: %s\n", ab->messengers[i]);
    }
    printf("-------------------------\n");
}

int contains_ignore_case(const char* haystack, const char* needle) {
    if (!haystack || !needle) return 0;
    if (!*needle) return 1;
    wchar_t wh[256], wn[256];
    if (mbstowcs(wh, haystack, 255) == (size_t)-1) return 0;
    if (mbstowcs(wn, needle, 255) == (size_t)-1) return 0;
    for (int i = 0; wh[i] != L'\0'; i++) wh[i] = towlower((wint_t)wh[i]);
    for (int i = 0; wn[i] != L'\0'; i++) wn[i] = towlower((wint_t)wn[i]);
    return wcsstr(wh, wn) != NULL;
}

void addAbonent(void) {
    if (count >= MAX_ABONENTS) {
        printf("\nОшибка: Телефонная книга переполнена!\n");
        return;
    }
    struct abonent* newAb = &database[count];
    char temp[MAX_STR];
    clearEntry(newAb);
    printf("\n--- Добавление абонента #%d ---\n", count + 1);
    while (1) {
        printf("Имя (только буквы): ");
        if (fgets(temp, sizeof(temp), stdin) == NULL) return;
        removeNewline(temp);
        if (isValidName(temp)) { strncpy(newAb->first_name, temp, MAX_STR - 1); break; }
        printf("Ошибка: Имя должно содержать только буквы и пробелы!\n");
    }
    while (1) {
        printf("Фамилия (только буквы): ");
        if (fgets(temp, sizeof(temp), stdin) == NULL) return;
        removeNewline(temp);
        if (isValidName(temp)) { strncpy(newAb->second_name, temp, MAX_STR - 1); break; }
        printf("Ошибка: Фамилия должна содержать только буквы и пробелы!\n");
    }
    printf("Отчество (Enter - пропустить): "); fgets(newAb->patronymic, MAX_STR, stdin); removeNewline(newAb->patronymic);
    printf("Место работы (Enter - пропустить): "); fgets(newAb->workplace, MAX_STR, stdin); removeNewline(newAb->workplace);
    printf("Должность (Enter - пропустить): "); fgets(newAb->position, MAX_STR, stdin); removeNewline(newAb->position);
    for (int i = 0; i < MAX_PHONES; i++) {
        printf("Телефон #%d (Enter - завершить ввод): ", i + 1);
        fgets(temp, sizeof(temp), stdin); removeNewline(temp);
        if (strlen(temp) == 0) break;
        if (isValidPhone(temp)) strncpy(newAb->phones[i], temp, MAX_STR - 1);
        else printf("Неверный формат, пропускаем.\n");
    }
    for (int i = 0; i < MAX_EMAILS; i++) {
        printf("Email #%d (Enter - завершить ввод): ", i + 1);
        fgets(temp, sizeof(temp), stdin); removeNewline(temp);
        if (strlen(temp) == 0) break;
        strncpy(newAb->emails[i], temp, MAX_STR - 1);
    }
    for (int i = 0; i < MAX_SOCIALS; i++) {
        printf("Соцсеть #%d (Enter - завершить ввод): ", i + 1);
        fgets(temp, sizeof(temp), stdin); removeNewline(temp);
        if (strlen(temp) == 0) break;
        strncpy(newAb->socials[i], temp, MAX_STR - 1);
    }
    for (int i = 0; i < MAX_MESSENGERS; i++) {
        printf("Мессенджер #%d (Enter - завершить ввод): ", i + 1);
        fgets(temp, sizeof(temp), stdin); removeNewline(temp);
        if (strlen(temp) == 0) break;
        strncpy(newAb->messengers[i], temp, MAX_STR - 1);
    }
    count++;
    printf("Абонент успешно добавлен!\n");
}

void deleteAbonent(void) {
    if (count == 0) { printf("\nОшибка: Телефонная книга пуста!\n"); return; }
    printf("\n--- Удаление абонента ---\n");
    printf("Введите номер записи для удаления (1-%d): ", count);
    int num;
    if (scanf("%d", &num) != 1) { printf("Ошибка ввода!\n"); clearInputBuffer(); return; }
    clearInputBuffer();
    if (num < 1 || num > count) { printf("Ошибка: Неверный номер записи!\n"); return; }
    for (int i = num - 1; i < count - 1; i++) {
        database[i] = database[i + 1];
    }
    clearEntry(&database[count - 1]);
    count--;
    printf("Абонент #%d успешно удален!\n", num);
}

void editAbonent(void) {
    if (count == 0) { printf("\nОшибка: Телефонная книга пуста!\n"); return; }
    printf("\n--- Редактирование записи ---\n");
    printf("Введите номер записи для редактирования (1-%d): ", count);
    int num;
    if (scanf("%d", &num) != 1) { printf("Ошибка ввода!\n"); clearInputBuffer(); return; }
    clearInputBuffer();
    if (num < 1 || num > count) { printf("Ошибка: Неверный номер записи!\n"); return; }
    struct abonent* target = &database[num - 1];
    char temp[MAX_STR];
    printf("\nРедактирование записи #%d (Enter - оставить без изменений)\n", num);
    printf("Имя (было: %s): ", target->first_name);
    fgets(temp, sizeof(temp), stdin); removeNewline(temp);
    if (strlen(temp) > 0 && !isOnlySpaces(temp)) {
        if (isValidName(temp)) strncpy(target->first_name, temp, MAX_STR - 1);
        else printf("Неверный формат имени, не изменено.\n");
    }
    printf("Фамилия (было: %s): ", target->second_name);
    fgets(temp, sizeof(temp), stdin); removeNewline(temp);
    if (strlen(temp) > 0 && !isOnlySpaces(temp)) {
        if (isValidName(temp)) strncpy(target->second_name, temp, MAX_STR - 1);
        else printf("Неверный формат фамилии, не изменено.\n");
    }
    printf("Отчество (было: %s): ", target->patronymic);
    fgets(temp, sizeof(temp), stdin); removeNewline(temp);
    if (strlen(temp) > 0) strncpy(target->patronymic, temp, MAX_STR - 1);
    printf("Место работы (было: %s): ", target->workplace);
    fgets(temp, sizeof(temp), stdin); removeNewline(temp);
    if (strlen(temp) > 0) strncpy(target->workplace, temp, MAX_STR - 1);
    printf("Должность (было: %s): ", target->position);
    fgets(temp, sizeof(temp), stdin); removeNewline(temp);
    if (strlen(temp) > 0) strncpy(target->position, temp, MAX_STR - 1);
    printf("Основной телефон (было: %s): ", target->phones[0]);
    fgets(temp, sizeof(temp), stdin); removeNewline(temp);
    if (strlen(temp) > 0) {
        if (isValidPhone(temp)) strncpy(target->phones[0], temp, MAX_STR - 1);
        else printf("Неверный формат телефона, не изменено.\n");
    }
    printf("Запись успешно обновлена!\n");
}

void searchByName(void) {
    if (count == 0) { printf("\nОшибка: Телефонная книга пуста!\n"); return; }
    char searchName[MAX_STR];
    printf("\n--- Поиск по имени/фамилии ---\n");
    printf("Введите имя или фамилию для поиска: ");
    if (fgets(searchName, sizeof(searchName), stdin) == NULL) return;
    searchName[strcspn(searchName, "\r\n")] = '\0';
    if (searchName[0] == '\0' || isOnlySpaces(searchName)) {
        printf("Ошибка: Строка поиска не может быть пустой!\n");
        return;
    }
    int found = 0;
    printf("\n=== Результаты поиска ===\n");
    for (int i = 0; i < count; i++) {
        if (contains_ignore_case(database[i].first_name, searchName) || 
            contains_ignore_case(database[i].second_name, searchName)) {
            printAbonent(&database[i], i + 1);
            found++;
        }
    }
    if (found == 0) {
        printf("Абоненты, содержащие '%s' в имени или фамилии, не найдены.\n", searchName);
    } else {
        printf("Найдено записей: %d\n", found);
    }
}

void displayAll(void) {
    if (count == 0) { printf("\nТелефонная книга пуста.\n"); return; }
    printf("\n=== ВСЕ ЗАПИСИ (%d из %d) ===\n", count, MAX_ABONENTS);
    for (int i = 0; i < count; i++) {
        printAbonent(&database[i], i + 1);
    }
    printf("=========================================\n");
}

void showMenu(void) {
    printf("\n========================================\n");
    printf("       ТЕЛЕФОННАЯ КНИГА                 \n");
    printf("========================================\n");
    printf(" 1) Добавить абонента                   \n");
    printf(" 2) Удалить абонента                    \n");
    printf(" 3) Редактировать абонента              \n");
    printf(" 4) Поиск по имени/фамилии              \n");
    printf(" 5) Показать все записи                 \n");
    printf(" 6) Выход                               \n");
    printf("========================================\n");
    printf("Выберите пункт меню (1-6): ");
}

int addAbonentDirect(const struct abonent* new_ab) {
    if (count >= MAX_ABONENTS) return -1;
    if (!isValidName(new_ab->first_name) || !isValidName(new_ab->second_name)) return -1;
    database[count] = *new_ab;
    return count++;
}

int deleteAbonentByIndex(int index) {
    if (index < 0 || index >= count) return -1;
    for (int i = index; i < count - 1; i++) {
        database[i] = database[i + 1];
    }
    clearEntry(&database[count - 1]);
    count--;
    return 0;
}

int findAbonentByName(const char* name) {
    for (int i = 0; i < count; i++) {
        if (contains_ignore_case(database[i].first_name, name) || 
            contains_ignore_case(database[i].second_name, name)) {
            return i;
        }
    }
    return -1;
}