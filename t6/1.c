#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

struct abonent {
    char name[30];
    char second_name[30];
    char tel[20];
    struct abonent* prev;  // указатель на предыдущий узел
    struct abonent* next;  // указатель на следующий узел
};

struct abonent* head = NULL;  // начало списка
struct abonent* tail = NULL;  // конец списка
int count = 0;                 // количество абонентов

// функции
void removeNewline(char* str);
void clearInputBuffer();
int strcmpIgnoreCase(const char* s1, const char* s2);
int isOnlySpaces(const char* str);
int isValidName(const char* name);
int isValidPhone(const char* phone);

void addAbonent();
void deleteAbonent();
void searchByName();
void displayAll();
void freeList();
void showMenu();

// главная функция
int main() {
    int choice;

    while (1) {
        showMenu();
        
        if (scanf("%d", &choice) != 1) {
            printf("Input error! Enter a number from 1 to 5.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1: addAbonent(); break;
            case 2: deleteAbonent(); break;
            case 3: searchByName(); break;
            case 4: displayAll(); break;
            case 5:
                printf("\nExit from program. Goodbye!\n");
                freeList();  // освобождение всей памяти
                return 0;
            default:
                printf("Error: Enter a number from 1 to 5!\n");
        }
    }
    return 0;
}

// реализация вспом. функций

void removeNewline(char* str) {
    char* newline = strchr(str, '\n');
    if (newline) *newline = '\0';
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// сравнение строк без учёта регистра
int strcmpIgnoreCase(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = tolower((unsigned char)*s1);
        char c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++; s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
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
        if (isdigit((unsigned char)c)) {
            hasDigit = 1;
        } else if (c != '+' && c != '-' && c != ' ' && c != '(' && c != ')') {
            return 0;
        }
        phone++;
    }
    return hasDigit;
}

// добавляем абонента + память под 1 узел
void addAbonent() {
    // выеделение памяти
    struct abonent* newAb = (struct abonent*)malloc(sizeof(struct abonent));
    if (newAb == NULL) {
        printf("Error: Failed to allocate memory!\n");
        return;
    }

    char temp[30];

    printf("\n--- Add New Abonent #%d ---\n", count + 1);
    
    // ввод имени
    while (1) {
        printf("Enter name (letters only, max 29 chars): ");
        if (fgets(temp, sizeof(temp), stdin) == NULL) { free(newAb); return; }
        removeNewline(temp);
        if (isValidName(temp)) {
            strncpy(newAb->name, temp, sizeof(newAb->name) - 1);
            newAb->name[sizeof(newAb->name) - 1] = '\0';
            break;
        }
        printf("Error: Name must contain only letters and spaces!\n");
    }

    // ввод фамилии
    while (1) {
        printf("Enter second name (letters only, max 29 chars): ");
        if (fgets(temp, sizeof(temp), stdin) == NULL) { free(newAb); return; }
        removeNewline(temp);
        if (isValidName(temp)) {
            strncpy(newAb->second_name, temp, sizeof(newAb->second_name) - 1);
            newAb->second_name[sizeof(newAb->second_name) - 1] = '\0';
            break;
        }
        printf("Error: Second name must contain only letters and spaces!\n");
    }

    // телефон
    while (1) {
        printf("Enter phone number (max 19 chars): ");
        if (fgets(newAb->tel, sizeof(newAb->tel), stdin) == NULL) { free(newAb); return; }
        removeNewline(newAb->tel);
        if (isValidPhone(newAb->tel)) break;
        printf("Error: Invalid phone number! Use digits, +, -, spaces, or parentheses.\n");
    }

    // вставка в конец
    newAb->next = NULL;
    newAb->prev = tail;
    
    if (tail != NULL) {
        tail->next = newAb;  // старый последний переходит в новый
    } else {
        head = newAb;  // если список пуст это 1й элемент
    }
    tail = newAb;  // новый становится последним
    
    count++;
    printf("Abonent successfully added!\n");
}

// удаление
void deleteAbonent() {
    if (count == 0) {
        printf("\nError: Phonebook is empty!\n");
        return;
    }

    printf("\n--- Delete Abonent ---\n");
    printf("Enter record number to delete (1-%d): ", count);
    
    int num;
    if (scanf("%d", &num) != 1) {
        printf("Input error!\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (num < 1 || num > count) {
        printf("Error: Invalid record number!\n");
        return;
    }

    // обход списка
    struct abonent* current = head;
    for (int i = 1; i < num && current != NULL; i++) {
        current = current->next;
    }
    
    if (current == NULL) {
        printf("Error: Abonent not found!\n");
        return;
    }

    // обход удаляемого узла
    if (current->prev != NULL) {
        current->prev->next = current->next;
    } else {
        head = current->next;  // удалили первый элемент
    }
    
    if (current->next != NULL) {
        current->next->prev = current->prev;
    } else {
        tail = current->prev;  // удалили последний элемент
    }

    free(current);  // освобождение памяти именно этого узла
    count--;
    printf("Abonent #%d successfully deleted!\n", num);
}

// обход списка по указателям
void searchByName() {
    if (count == 0) {
        printf("\nError: Phonebook is empty!\n");
        return;
    }

    char searchName[30];
    printf("\n--- Search by Name ---\n");
    printf("Enter name to search: ");
    if (fgets(searchName, sizeof(searchName), stdin) == NULL) return;
    removeNewline(searchName);

    if (searchName[0] == '\0' || isOnlySpaces(searchName)) {
        printf("Error: Search name cannot be empty!\n");
        return;
    }

    int found = 0;
    int index = 1;
    struct abonent* current = head;  // начало с головы
    
    printf("\n=== Search Results ===\n");
    while (current != NULL) {  // обход через указатели
        if (strcmpIgnoreCase(current->name, searchName) == 0) {
            printf("Record #%d:\n", index);
            printf("  Name: %s\n", current->name);
            printf("  Second Name: %s\n", current->second_name);
            printf("  Phone: %s\n", current->tel);
            printf("-------------------------\n");
            found++;
        }
        current = current->next;  // переход
        index++;
    }

    if (found == 0) {
        printf("No abonents found with name '%s'.\n", searchName);
    } else {
        printf("Records found: %d\n", found);
    }
}

// все записи
void displayAll() {
    if (count == 0) {
        printf("\nPhonebook is empty.\n");
        return;
    }

    printf("\n=== ALL RECORDS (%d total) ===\n", count);
    printf("%-4s %-20s %-20s %-15s\n", "#", "Name", "Second Name", "Phone");
    printf("------------------------------------------------------------------\n");
    
    struct abonent* current = head;
    int index = 1;
    
    while (current != NULL) {  // обход через указатели
        const char* name = (current->name[0] != '\0') ? current->name : "[empty]";
        const char* sname = (current->second_name[0] != '\0') ? current->second_name : "[empty]";
        const char* phone = (current->tel[0] != '\0') ? current->tel : "[empty]";
        
        printf("%-4d %-20s %-20s %-15s\n", index, name, sname, phone);
        
        current = current->next;  // переход к следующему
        index++;
    }
    printf("==================================================================\n");
}

// очищаем всю память
void freeList() {
    struct abonent* current = head;
    struct abonent* nextNode;
    
    while (current != NULL) {
        nextNode = current->next;  // сохраняем указатель на следующий
        free(current);              // освобождаем текущий узел
        current = nextNode;         // переходим к следующему
    }
    
    head = NULL;
    tail = NULL;
    count = 0;
}

// === Меню ===
void showMenu() {
    printf("\n========================================\n");
    printf("       PHONEBOOK DIRECTORY            \n");
    printf("========================================\n");
    printf(" 1) Add abonent                       \n");
    printf(" 2) Delete abonent                    \n");
    printf(" 3) Search abonents by name           \n");
    printf(" 4) Display all records               \n");
    printf(" 5) Exit                              \n");
    printf("========================================\n");
    printf("Select menu item (1-5): ");
}