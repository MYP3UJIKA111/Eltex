#include "spravochnik.h"
#include <strings.h>

struct TreeNode* root = NULL;
int next_id = 1;

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int strcmpIgnoreCase(const char* s1, const char* s2) {
    return strcasecmp(s1, s2);
}

int isValidName(const char* name) {
    if (name[0] == '\0') return 0;
    while (*name) {
        unsigned char c = (unsigned char)*name;
        if (c > 127) { name++; continue; }
        if (!isalpha(c) && !isspace(c)) return 0;
        name++;
    }
    return 1;
}

int isValidPhone(const char* phone) {
    if (phone[0] == '\0') return 0;
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
    if (!haystack || !needle || !*needle) return 0;
    wchar_t wh[256], wn[256];
    if (mbstowcs(wh, haystack, 255) == (size_t)-1) return 0;
    if (mbstowcs(wn, needle, 255) == (size_t)-1) return 0;
    for (int i = 0; wh[i] != L'\0'; i++) wh[i] = towlower((wint_t)wh[i]);
    for (int i = 0; wn[i] != L'\0'; i++) wn[i] = towlower((wint_t)wn[i]);
    return wcsstr(wh, wn) != NULL;
}

/* --- АВЛ ДЕРЕВО: БАЛАНСИРОВКА С ВИЗУАЛИЗАЦИЕЙ --- */

int getHeight(struct TreeNode* node) {
    return node == NULL ? 0 : node->height;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

struct TreeNode* createNode(struct abonent data) {
    struct TreeNode* node = (struct TreeNode*)malloc(sizeof(struct TreeNode));
    node->data = data;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

struct TreeNode* rightRotate(struct TreeNode* y) {
    printf("\n[АВЛ-БАЛАНСИРОВКА] Выполнен ПРАВЫЙ поворот (LL) вокруг узла: %s %s\n", 
           y->data.second_name, y->data.first_name);
    struct TreeNode* x = y->left;
    struct TreeNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    return x;
}

struct TreeNode* leftRotate(struct TreeNode* x) {
    printf("\n[АВЛ-БАЛАНСИРОВКА] Выполнен ЛЕВЫЙ поворот (RR) вокруг узла: %s %s\n", 
           x->data.second_name, x->data.first_name);
    struct TreeNode* y = x->right;
    struct TreeNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    return y;
}

int getBalance(struct TreeNode* node) {
    if (node == NULL) return 0;
    return getHeight(node->left) - getHeight(node->right);
}

struct TreeNode* insert(struct TreeNode* node, struct abonent data) {
    if (node == NULL) return createNode(data);

    int cmp = strcmpIgnoreCase(data.second_name, node->data.second_name);
    if (cmp == 0) cmp = strcmpIgnoreCase(data.first_name, node->data.first_name);

    if (cmp < 0)
        node->left = insert(node->left, data);
    else if (cmp > 0)
        node->right = insert(node->right, data);
    else
        return node;

    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    int balance = getBalance(node);

    if (balance > 1 && strcmpIgnoreCase(data.second_name, node->left->data.second_name) < 0)
        return rightRotate(node);
    if (balance < -1 && strcmpIgnoreCase(data.second_name, node->right->data.second_name) > 0)
        return leftRotate(node);
    if (balance > 1 && strcmpIgnoreCase(data.second_name, node->left->data.second_name) > 0) {
        printf("\n[АВЛ-БАЛАНСИРОВКА] Выполнен двойной поворот (LR) для узла: %s %s\n", node->data.second_name, node->data.first_name);
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if (balance < -1 && strcmpIgnoreCase(data.second_name, node->right->data.second_name) < 0) {
        printf("\n[АВЛ-БАЛАНСИРОВКА] Выполнен двойной поворот (RL) для узла: %s %s\n", node->data.second_name, node->data.first_name);
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }
    return node;
}

struct TreeNode* minValueNode(struct TreeNode* node) {
    struct TreeNode* current = node;
    while (current->left != NULL) current = current->left;
    return current;
}

struct TreeNode* deleteNode(struct TreeNode* root, const char* surname, const char* firstname) {
    if (root == NULL) return root;

    int cmp_s = strcmpIgnoreCase(surname, root->data.second_name);
    int cmp = (cmp_s == 0) ? strcmpIgnoreCase(firstname, root->data.first_name) : cmp_s;

    if (cmp < 0) root->left = deleteNode(root->left, surname, firstname);
    else if (cmp > 0) root->right = deleteNode(root->right, surname, firstname);
    else {
        if ((root->left == NULL) || (root->right == NULL)) {
            struct TreeNode* temp = root->left ? root->left : root->right;
            if (temp == NULL) {
                temp = root;
                root = NULL;
            } else {
                *root = *temp;
            }
            free(temp);
        } else {
            struct TreeNode* temp = minValueNode(root->right);
            root->data = temp->data;
            root->right = deleteNode(root->right, temp->data.second_name, temp->data.first_name);
        }
    }

    if (root == NULL) return root;

    root->height = 1 + max(getHeight(root->left), getHeight(root->right));
    int balance = getBalance(root);

    if (balance > 1 && getBalance(root->left) >= 0) return rightRotate(root);
    if (balance > 1 && getBalance(root->left) < 0) {
        printf("\n[АВЛ-БАЛАНСИРОВКА] Выполнен двойной поворот (LR) при удалении для узла: %s %s\n", root->data.second_name, root->data.first_name);
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    if (balance < -1 && getBalance(root->right) <= 0) return leftRotate(root);
    if (balance < -1 && getBalance(root->right) > 0) {
        printf("\n[АВЛ-БАЛАНСИРОВКА] Выполнен двойной поворот (RL) при удалении для узла: %s %s\n", root->data.second_name, root->data.first_name);
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }
    return root;
}

struct TreeNode* searchExact(struct TreeNode* root, const char* surname, const char* firstname) {
    if (root == NULL) return NULL;
    int cmp_s = strcmpIgnoreCase(surname, root->data.second_name);
    int cmp = (cmp_s == 0) ? strcmpIgnoreCase(firstname, root->data.first_name) : cmp_s;
    
    if (cmp == 0) return root;
    if (cmp < 0) return searchExact(root->left, surname, firstname);
    return searchExact(root->right, surname, firstname);
}

void searchByName(struct TreeNode* root, const char* name) {
    if (root == NULL) return;
    searchByName(root->left, name);
    if (contains_ignore_case(root->data.first_name, name) || contains_ignore_case(root->data.second_name, name)) {
        printf("  [ID: %d] %s %s %s\n", root->data.id, root->data.second_name, root->data.first_name, root->data.patronymic);
    }
    searchByName(root->right, name);
}

void printAbonentData(const struct abonent* ab) {
    printf("  [ID: %d] %s %s %s\n", ab->id, ab->second_name, ab->first_name, ab->patronymic);
    if (ab->workplace[0] || ab->position[0]) printf("    Работа: %s, Должность: %s\n", ab->workplace, ab->position);
    for (int i = 0; i < MAX_PHONES; i++) if (ab->phones[i][0] != '\0') printf("    Тел: %s\n", ab->phones[i]);
    printf("  -------------------------\n");
}

void printInOrder(struct TreeNode* root) {
    if (root != NULL) {
        printInOrder(root->left);
        printAbonentData(&root->data);
        printInOrder(root->right);
    }
}

void printTreeVisual(struct TreeNode* root, int space) {
    if (root == NULL) return;
    space += 10;
    printTreeVisual(root->right, space);
    printf("\n");
    for (int i = 10; i < space; i++) printf(" ");
    printf("[ID:%d] %s %s\n", root->data.id, root->data.second_name, root->data.first_name);
    printTreeVisual(root->left, space);
}

void freeTree(struct TreeNode* root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

/* --- ФУНКЦИИ МЕНЮ С ЗАЩИЩЕННЫМ ВВОДОМ --- */

void addAbonent(void) {
    struct abonent newAb;
    char temp[MAX_STR];
    memset(&newAb, 0, sizeof(struct abonent));
    
    newAb.id = next_id++;
    printf("\n--- Добавление абонента (будет присвоен ID: %d) ---\n", newAb.id);
    
    while (1) {
        printf("Имя (только буквы): ");
        if (!fgets(temp, sizeof(temp), stdin)) continue;
        temp[strcspn(temp, "\n")] = '\0';
        if (isValidName(temp)) { strncpy(newAb.first_name, temp, MAX_STR - 1); break; }
        printf("Ошибка: Имя должно содержать только буквы и пробелы!\n");
    }
    while (1) {
        printf("Фамилия (только буквы): ");
        if (!fgets(temp, sizeof(temp), stdin)) continue;
        temp[strcspn(temp, "\n")] = '\0';
        if (isValidName(temp)) { strncpy(newAb.second_name, temp, MAX_STR - 1); break; }
        printf("Ошибка: Фамилия должна содержать только буквы и пробелы!\n");
    }
    
    printf("Отчество (Enter - пропустить): "); fgets(newAb.patronymic, MAX_STR, stdin); newAb.patronymic[strcspn(newAb.patronymic, "\n")] = '\0';
    printf("Место работы (Enter - пропустить): "); fgets(newAb.workplace, MAX_STR, stdin); newAb.workplace[strcspn(newAb.workplace, "\n")] = '\0';
    printf("Должность (Enter - пропустить): "); fgets(newAb.position, MAX_STR, stdin); newAb.position[strcspn(newAb.position, "\n")] = '\0';
    
    for (int i = 0; i < MAX_PHONES; i++) {
        printf("Телефон #%d (Enter - завершить): ", i + 1);
        fgets(temp, sizeof(temp), stdin); temp[strcspn(temp, "\n")] = '\0';
        if (strlen(temp) == 0) break;
        if (isValidPhone(temp)) strncpy(newAb.phones[i], temp, MAX_STR - 1);
        else printf("Неверный формат.\n");
    }
    
    root = insert(root, newAb);
    printf("\nАбонент [ID: %d] успешно добавлен в АВЛ-дерево!\n", newAb.id);
}

void deleteAbonent(void) {
    char surname[MAX_STR], firstname[MAX_STR];
    printf("\n--- Удаление абонента ---\n");
    printf("Введите фамилию: ");
    if (!fgets(surname, sizeof(surname), stdin)) return;
    surname[strcspn(surname, "\n")] = '\0';
    
    printf("Введите имя: ");
    if (!fgets(firstname, sizeof(firstname), stdin)) return;
    firstname[strcspn(firstname, "\n")] = '\0';

    if (searchExact(root, surname, firstname) == NULL) {
        printf("Абонент не найден!\n");
        return;
    }
    root = deleteNode(root, surname, firstname);
    printf("Абонент успешно удален!\n");
}

void editAbonent(void) {
    char surname[MAX_STR], firstname[MAX_STR];
    printf("\n--- Редактирование абонента ---\n");
    printf("Введите текущую фамилию: ");
    if (!fgets(surname, sizeof(surname), stdin)) return;
    surname[strcspn(surname, "\n")] = '\0';
    
    printf("Введите текущее имя: ");
    if (!fgets(firstname, sizeof(firstname), stdin)) return;
    firstname[strcspn(firstname, "\n")] = '\0';

    struct TreeNode* node = searchExact(root, surname, firstname);
    if (node == NULL) {
        printf("Абонент не найден!\n");
        return;
    }

    printf("\nТекущие данные [ID: %d]:\n", node->data.id);
    printAbonentData(&node->data);
    printf("Введите новые данные (Enter - оставить как есть)\n");
    
    char temp[MAX_STR];
    printf("Новое имя (было: %s): ", node->data.first_name);
    fgets(temp, sizeof(temp), stdin); temp[strcspn(temp, "\n")] = '\0';
    if (strlen(temp) > 0 && isValidName(temp)) strncpy(node->data.first_name, temp, MAX_STR - 1);

    printf("Новая фамилия (было: %s): ", node->data.second_name);
    fgets(temp, sizeof(temp), stdin); temp[strcspn(temp, "\n")] = '\0';
    if (strlen(temp) > 0 && isValidName(temp)) strncpy(node->data.second_name, temp, MAX_STR - 1);

    if (strcmpIgnoreCase(surname, node->data.second_name) != 0 || strcmpIgnoreCase(firstname, node->data.first_name) != 0) {
        struct abonent updatedData = node->data;
        printf("\n[!] Ключевые поля изменены. Узел будет перемещен в новую позицию дерева для сохранения сортировки.\n");
        root = deleteNode(root, surname, firstname);
        root = insert(root, updatedData);
    } else {
        printf("Данные обновлены.\n");
    }
}

void searchMenu(void) {
    char name[MAX_STR];
    printf("\n--- Поиск по имени/фамилии ---\n");
    printf("Введите часть имени или фамилии: ");
    if (!fgets(name, sizeof(name), stdin)) return;
    name[strcspn(name, "\n")] = '\0';
    if (strlen(name) == 0) return;

    printf("\n=== Результаты поиска ===\n");
    searchByName(root, name);
    printf("=========================\n");
}

void displayAll(void) {
    printf("\n=== ВСЕ ЗАПИСИ (отсортированы по алфавиту) ===\n");
    if (root == NULL) {
        printf("Телефонная книга пуста.\n");
    } else {
        printInOrder(root);
    }
    printf("================================================\n");
}

void displayAsTree(void) {
    printf("\n=== ВИЗУАЛЬНОЕ ПРЕДСТАВЛЕНИЕ ДЕРЕВА ===\n");
    printf("(Правое поддерево сверху, левое снизу)\n");
    if (root == NULL) {
        printf("Дерево пусто.\n");
    } else {
        printTreeVisual(root, 0);
    }
    printf("\n=======================================\n");
}

void showMenu(void) {
    printf("\n========================================\n");
    printf("  ТЕЛЕФОННАЯ КНИГА (АВЛ-ДЕРЕВО)\n");
    printf("========================================\n");
    printf(" 1) Добавить абонента\n");
    printf(" 2) Удалить абонента (по Фамилии и Имени)\n");
    printf(" 3) Редактировать абонента (по Фамилии и Имени)\n");
    printf(" 4) Поиск по имени/фамилии\n");
    printf(" 5) Показать все записи (по алфавиту)\n");
    printf(" 6) Показать структуру дерева\n");
    printf(" 7) Выход\n");
    printf("========================================\n");
    printf("Выберите пункт меню (1-7): ");
}