#include "spravochnik.h"
#include <strings.h>

struct Node* head = NULL;
int count = 0;
int next_id = 1;

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

int compareAbonents(const struct abonent* a, const struct abonent* b) {
    int cmp = strcasecmp(a->second_name, b->second_name);
    if (cmp != 0) return cmp;
    return strcasecmp(a->first_name, b->first_name);
}

struct Node* createNode(const struct abonent* data) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (newNode == NULL) {
        fprintf(stderr, "Ошибка выделения памяти!\n");
        return NULL;
    }
    newNode->data = *data;
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}

struct Node* insertSorted(struct Node* head, const struct abonent* data) {
    struct Node* newNode = createNode(data);
    if (newNode == NULL) return head;

    if (head == NULL) {
        newNode->next = newNode;
        newNode->prev = newNode;
        return newNode;
    }

    struct Node* tmp = head;

    do {
        if (compareAbonents(&newNode->data, &tmp->data) <= 0) {
            newNode->next = tmp;
            newNode->prev = tmp->prev;
            tmp->prev->next = newNode;
            tmp->prev = newNode;
            if (tmp == head) {
                head = newNode;
            }
            return head;
        }
        tmp = tmp->next;
    } while (tmp != head);

    newNode->next = head;
    newNode->prev = head->prev;
    head->prev->next = newNode;
    head->prev = newNode;

    return head;
}

struct Node* getNodeById(int id) {
    if (head == NULL) return NULL;
    struct Node* tmp = head;
    do {
        if (tmp->data.id == id) return tmp;
        tmp = tmp->next;
    } while (tmp != head);
    return NULL;
}

struct Node* deleteNodeById(struct Node* head, int id) {
    if (head == NULL) return NULL;

    struct Node* tmp = head;

    do {
        if (tmp->data.id == id) {
            if (tmp->next == tmp && tmp->prev == tmp) {
                free(tmp);
                return NULL;
            }
            tmp->next->prev = tmp->prev;
            tmp->prev->next = tmp->next;
            if (tmp == head) {
                head = head->next;
            }
            free(tmp);
            return head;
        }
        tmp = tmp->next;
    } while (tmp != head);

    return head;
}

struct Node* deleteList(struct Node* head) {
    if (head == NULL) return NULL;
    struct Node* tmp = head;
    struct Node* itemForRemove;
    do {
        itemForRemove = tmp;
        tmp = tmp->next;
        free(itemForRemove);
    } while (tmp != head);
    return NULL;
}

void printAbonentNode(const struct Node* node) {
    printf("  [ID: %d]\n", node->data.id);
    printf("  ФИО: %s %s %s\n",
           node->data.second_name,
           node->data.first_name,
           node->data.patronymic);
    printf("  Работа: %s, Должность: %s\n",
           node->data.workplace,
           node->data.position);
    for (int i = 0; i < MAX_PHONES; i++) {
        if (node->data.phones[i][0] != '\0')
            printf("  Телефон: %s\n", node->data.phones[i]);
    }
    for (int i = 0; i < MAX_EMAILS; i++) {
        if (node->data.emails[i][0] != '\0')
            printf("  Email: %s\n", node->data.emails[i]);
    }
    for (int i = 0; i < MAX_SOCIALS; i++) {
        if (node->data.socials[i][0] != '\0')
            printf("  Соцсети: %s\n", node->data.socials[i]);
    }
    for (int i = 0; i < MAX_MESSENGERS; i++) {
        if (node->data.messengers[i][0] != '\0')
            printf("  Мессенджер: %s\n", node->data.messengers[i]);
    }
    printf("-------------------------\n");
}

void printList(void) {
    if (head == NULL) {
        printf("\nТелефонная книга пуста.\n");
        return;
    }
    struct Node* tmp = head;
    do {
        printAbonentNode(tmp);
        tmp = tmp->next;
    } while (tmp != head);
}

void addAbonent(void) {
    struct abonent newAb;
    char temp[MAX_STR];
    clearEntry(&newAb);

    newAb.id = next_id++;

    printf("\n--- Добавление абонента (ID: %d) ---\n", newAb.id);

    while (1) {
        printf("Имя (только буквы): ");
        if (fgets(temp, sizeof(temp), stdin) == NULL) return;
        removeNewline(temp);
        if (isValidName(temp)) {
            strncpy(newAb.first_name, temp, MAX_STR - 1);
            break;
        }
        printf("Ошибка: Имя должно содержать только буквы и пробелы!\n");
    }

    while (1) {
        printf("Фамилия (только буквы): ");
        if (fgets(temp, sizeof(temp), stdin) == NULL) return;
        removeNewline(temp);
        if (isValidName(temp)) {
            strncpy(newAb.second_name, temp, MAX_STR - 1);
            break;
        }
        printf("Ошибка: Фамилия должна содержать только буквы и пробелы!\n");
    }

    printf("Отчество (Enter - пропустить): ");
    fgets(newAb.patronymic, MAX_STR, stdin);
    removeNewline(newAb.patronymic);

    printf("Место работы (Enter - пропустить): ");
    fgets(newAb.workplace, MAX_STR, stdin);
    removeNewline(newAb.workplace);

    printf("Должность (Enter - пропустить): ");
    fgets(newAb.position, MAX_STR, stdin);
    removeNewline(newAb.position);

    for (int i = 0; i < MAX_PHONES; i++) {
        printf("Телефон #%d (Enter - завершить ввод): ", i + 1);
        fgets(temp, sizeof(temp), stdin);
        removeNewline(temp);
        if (strlen(temp) == 0) break;
        if (isValidPhone(temp))
            strncpy(newAb.phones[i], temp, MAX_STR - 1);
        else
            printf("Неверный формат, пропускаем.\n");
    }

    for (int i = 0; i < MAX_EMAILS; i++) {
        printf("Email #%d (Enter - завершить ввод): ", i + 1);
        fgets(temp, sizeof(temp), stdin);
        removeNewline(temp);
        if (strlen(temp) == 0) break;
        strncpy(newAb.emails[i], temp, MAX_STR - 1);
    }

    for (int i = 0; i < MAX_SOCIALS; i++) {
        printf("Соцсеть #%d (Enter - завершить ввод): ", i + 1);
        fgets(temp, sizeof(temp), stdin);
        removeNewline(temp);
        if (strlen(temp) == 0) break;
        strncpy(newAb.socials[i], temp, MAX_STR - 1);
    }

    for (int i = 0; i < MAX_MESSENGERS; i++) {
        printf("Мессенджер #%d (Enter - завершить ввод): ", i + 1);
        fgets(temp, sizeof(temp), stdin);
        removeNewline(temp);
        if (strlen(temp) == 0) break;
        strncpy(newAb.messengers[i], temp, MAX_STR - 1);
    }

    head = insertSorted(head, &newAb);
    count++;
    printf("Абонент успешно добавлен с ID: %d!\n", newAb.id);
}

void deleteAbonent(void) {
    if (count == 0) {
        printf("\nОшибка: Телефонная книга пуста!\n");
        return;
    }

    printf("\n--- Удаление абонента ---\n");
    printf("Введите ID абонента для удаления: ");

    int id;
    if (scanf("%d", &id) != 1) {
        printf("Ошибка ввода!\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    struct Node* target = getNodeById(id);
    if (target == NULL) {
        printf("Ошибка: Абонент с ID %d не найден!\n", id);
        return;
    }

    printf("Удаляется: [ID: %d] %s %s\n",
           target->data.id,
           target->data.second_name,
           target->data.first_name);

    head = deleteNodeById(head, id);
    count--;
    printf("Абонент с ID %d успешно удален!\n", id);
}

void editAbonent(void) {
    if (count == 0) {
        printf("\nОшибка: Телефонная книга пуста!\n");
        return;
    }

    printf("\n--- Редактирование записи ---\n");
    printf("Введите ID абонента для редактирования: ");

    int id;
    if (scanf("%d", &id) != 1) {
        printf("Ошибка ввода!\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    struct Node* target = getNodeById(id);
    if (target == NULL) {
        printf("Ошибка: Абонент с ID %d не найден!\n", id);
        return;
    }

    char temp[MAX_STR];
    printf("\nРедактирование [ID: %d] (Enter - оставить без изменений)\n", id);

    char old_second_name[MAX_STR];
    strncpy(old_second_name, target->data.second_name, MAX_STR - 1);
    old_second_name[MAX_STR - 1] = '\0';

    printf("Имя (было: %s): ", target->data.first_name);
    fgets(temp, sizeof(temp), stdin);
    removeNewline(temp);
    if (strlen(temp) > 0 && !isOnlySpaces(temp)) {
        if (isValidName(temp))
            strncpy(target->data.first_name, temp, MAX_STR - 1);
        else
            printf("Неверный формат имени, не изменено.\n");
    }

    printf("Фамилия (было: %s): ", target->data.second_name);
    fgets(temp, sizeof(temp), stdin);
    removeNewline(temp);
    if (strlen(temp) > 0 && !isOnlySpaces(temp)) {
        if (isValidName(temp))
            strncpy(target->data.second_name, temp, MAX_STR - 1);
        else
            printf("Неверный формат фамилии, не изменено.\n");
    }

    printf("Отчество (было: %s): ", target->data.patronymic);
    fgets(temp, sizeof(temp), stdin);
    removeNewline(temp);
    if (strlen(temp) > 0)
        strncpy(target->data.patronymic, temp, MAX_STR - 1);

    printf("Место работы (было: %s): ", target->data.workplace);
    fgets(temp, sizeof(temp), stdin);
    removeNewline(temp);
    if (strlen(temp) > 0)
        strncpy(target->data.workplace, temp, MAX_STR - 1);

    printf("Должность (было: %s): ", target->data.position);
    fgets(temp, sizeof(temp), stdin);
    removeNewline(temp);
    if (strlen(temp) > 0)
        strncpy(target->data.position, temp, MAX_STR - 1);

    printf("Основной телефон (было: %s): ", target->data.phones[0]);
    fgets(temp, sizeof(temp), stdin);
    removeNewline(temp);
    if (strlen(temp) > 0) {
        if (isValidPhone(temp))
            strncpy(target->data.phones[0], temp, MAX_STR - 1);
        else
            printf("Неверный формат телефона, не изменено.\n");
    }

    if (strcmp(old_second_name, target->data.second_name) != 0) {
        struct abonent data_copy = target->data;
        head = deleteNodeById(head, id);
        head = insertSorted(head, &data_copy);
        printf("\n[!] Фамилия изменена. Запись перемещена в новую позицию списка.\n");
    }

    printf("Запись [ID: %d] успешно обновлена!\n", id);
}

void searchByName(void) {
    if (count == 0) {
        printf("\nОшибка: Телефонная книга пуста!\n");
        return;
    }

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

    struct Node* tmp = head;
    do {
        if (contains_ignore_case(tmp->data.first_name, searchName) ||
            contains_ignore_case(tmp->data.second_name, searchName)) {
            printAbonentNode(tmp);
            found++;
        }
        tmp = tmp->next;
    } while (tmp != head);

    if (found == 0) {
        printf("Абоненты, содержащие '%s' в имени или фамилии, не найдены.\n", searchName);
    } else {
        printf("Найдено записей: %d\n", found);
    }
}

void searchById(void) {
    if (count == 0) {
        printf("\nОшибка: Телефонная книга пуста!\n");
        return;
    }

    printf("\n--- Поиск по ID ---\n");
    printf("Введите ID абонента: ");

    int id;
    if (scanf("%d", &id) != 1) {
        printf("Ошибка ввода!\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    struct Node* target = getNodeById(id);
    if (target == NULL) {
        printf("Абонент с ID %d не найден.\n", id);
        return;
    }

    printf("\n=== Результат поиска ===\n");
    printAbonentNode(target);
}

void displayAll(void) {
    if (count == 0) {
        printf("\nТелефонная книга пуста.\n");
        return;
    }
    printf("\n=== ВСЕ ЗАПИСИ (%d шт., упорядочены по ФИО) ===\n", count);
    printList();
    printf("=========================================\n");
}

void showMenu(void) {
    printf("\n========================================\n");
    printf("  ТЕЛЕФОННАЯ КНИГА (двусвязный список)\n");
    printf("========================================\n");
    printf(" 1) Добавить абонента                   \n");
    printf(" 2) Удалить абонента (по ID)            \n");
    printf(" 3) Редактировать абонента (по ID)      \n");
    printf(" 4) Поиск по имени/фамилии              \n");
    printf(" 5) Поиск по ID                         \n");
    printf(" 6) Показать все записи                 \n");
    printf(" 7) Выход                               \n");
    printf("========================================\n");
    printf("Выберите пункт меню (1-7): ");
}

int addAbonentDirect(const struct abonent* new_ab) {
    if (!isValidName(new_ab->first_name) || !isValidName(new_ab->second_name))
        return -1;
    struct abonent copy = *new_ab;
    copy.id = next_id++;
    head = insertSorted(head, &copy);
    count++;
    return 0;
}

int deleteAbonentByIndex(int index) {
    if (head == NULL || index < 0 || index >= count) return -1;
    struct Node* tmp = head;
    for (int i = 0; i < index; i++) {
        tmp = tmp->next;
    }
    int id = tmp->data.id;
    head = deleteNodeById(head, id);
    count--;
    return 0;
}

int findAbonentByName(const char* name) {
    if (head == NULL) return -1;
    struct Node* tmp = head;
    int i = 0;
    do {
        if (contains_ignore_case(tmp->data.first_name, name) ||
            contains_ignore_case(tmp->data.second_name, name)) {
            return tmp->data.id;
        }
        tmp = tmp->next;
        i++;
    } while (tmp != head);
    return -1;
}