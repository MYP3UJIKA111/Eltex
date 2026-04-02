#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_ABONENTS 100

struct abonent {
    char name[30];        // Имя
    char second_name[30]; // Фамилия
    char tel[20];         // Телефон
};

struct abonent database[MAX_ABONENTS];
int count = 0;

void removeNewline(char* str); // Функция удаления символа новой строки
void clearEntry(struct abonent* ab); // Функция очистки записи (заполнение нулями)
void clearInputBuffer(); // Функция очистки буфера ввода
int strcmpIgnoreCase(const char* s1, const char* s2); // Функция сравнения строк без учёта регистра
int isOnlySpaces(const char* str); // Проверка, содержит ли строка только пробелы
int isValidName(const char* name); // Валидация имени/фамилии: только буквы и пробелы
int isValidPhone(const char* phone); // Валидация телефонного номера
void addAbonent(); // Добавить абонента
void deleteAbonent(); // Удалить абонента
void searchByName(); // Поиск абонентов по имени (без учёта регистра)
void displayAll(); // Вывод всех записей
void showMenu(); // Вывод меню

// Главная функция
int main() {
    memset(database, 0, sizeof(database));

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
            case 1:
                addAbonent();
                break;
            case 2:
                deleteAbonent();
                break;
            case 3:
                searchByName();
                break;
            case 4:
                displayAll();
                break;
            case 5:
                printf("\nExit from program. Goodbye!\n");
                return 0;
            default:
                printf("Error: Enter a number from 1 to 5!\n");
        }
    }

    return 0;
}

void removeNewline(char* str) {
    char* newline = strchr(str, '\n');
    if (newline) *newline = '\0';
}

void clearEntry(struct abonent* ab) {
    memset(ab, 0, sizeof(struct abonent));
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int strcmpIgnoreCase(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = tolower((unsigned char)*s1);
        char c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
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
    // Проверка на пустую строку или только пробелы
    if (name[0] == '\0' || isOnlySpaces(name)) {
        return 0;
    }
    
    // Проверка каждого символа
    while (*name) {
        unsigned char c = (unsigned char)*name;
        // Разрешены: буквы (любой регистр) и пробелы
        // isalpha проверяет буквы в текущей локали
        if (!isalpha(c) && !isspace(c)) {
            return 0; // Найден недопустимый символ (цифра, знак препинания и т.д.)
        }
        name++;
    }
    return 1;
}

int isValidPhone(const char* phone) {
    if (phone[0] == '\0' || isOnlySpaces(phone)) {
        return 0; // Пустой номер не допускается
    }
    
    int hasDigit = 0;
    while (*phone) {
        char c = *phone;
        if (isdigit((unsigned char)c)) {
            hasDigit = 1;
        } else if (c != '+' && c != '-' && c != ' ' && c != '(' && c != ')') {
            return 0; // Недопустимый символ
        }
        phone++;
    }
    return hasDigit; // Должна быть хотя бы одна цифра
}

void addAbonent() {
    if (count >= MAX_ABONENTS) {
        printf("\nError: Phonebook is full (maximum %d records)!\n", MAX_ABONENTS);
        return;
    }

    struct abonent* newAb = &database[count];
    char temp[30];

    printf("\n--- Add New Abonent #%d ---\n", count + 1);
    
    // Ввод имени с валидацией
    while (1) {
        printf("Enter name (letters only, max 29 chars): ");
        if (fgets(temp, sizeof(temp), stdin) == NULL) return;
        removeNewline(temp);
        if (isValidName(temp)) {
            strncpy(newAb->name, temp, sizeof(newAb->name) - 1);
            newAb->name[sizeof(newAb->name) - 1] = '\0';
            break;
        }
        printf("Error: Name must contain only letters and spaces!\n");
    }

    // Ввод фамилии с валидацией
    while (1) {
        printf("Enter second name (letters only, max 29 chars): ");
        if (fgets(temp, sizeof(temp), stdin) == NULL) return;
        removeNewline(temp);
        if (isValidName(temp)) {
            strncpy(newAb->second_name, temp, sizeof(newAb->second_name) - 1);
            newAb->second_name[sizeof(newAb->second_name) - 1] = '\0';
            break;
        }
        printf("Error: Second name must contain only letters and spaces!\n");
    }

    // Ввод телефона с валидацией
    while (1) {
        printf("Enter phone number (max 19 chars): ");
        if (fgets(newAb->tel, sizeof(newAb->tel), stdin) == NULL) return;
        removeNewline(newAb->tel);
        
        if (isValidPhone(newAb->tel)) {
            break;
        }
        printf("Error: Invalid phone number! Use digits, +, -, spaces, or parentheses.\n");
    }

    count++;
    printf("Abonent successfully added!\n");
}

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

    clearEntry(&database[num - 1]);
    
    for (int i = num - 1; i < count - 1; i++) {
        database[i] = database[i + 1];
    }
    
    count--;
    printf("Abonent #%d successfully deleted!\n", num);
}

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
    printf("\n=== Search Results ===\n");
    
    for (int i = 0; i < count; i++) {
        if (strcmpIgnoreCase(database[i].name, searchName) == 0) {
            printf("Record #%d:\n", i + 1);
            printf("  Name: %s\n", database[i].name);
            printf("  Second Name: %s\n", database[i].second_name);
            printf("  Phone: %s\n", database[i].tel);
            printf("-------------------------\n");
            found++;
        }
    }

    if (found == 0) {
        printf("No abonents found with name '%s'.\n", searchName);
    } else {
        printf("Records found: %d\n", found);
    }
}

void displayAll() {
    if (count == 0) {
        printf("\nPhonebook is empty.\n");
        return;
    }

    printf("\n=== ALL RECORDS (%d of %d) ===\n", count, MAX_ABONENTS);
    printf("%-4s %-20s %-20s %-15s\n", "#", "Name", "Second Name", "Phone");
    printf("------------------------------------------------------------------\n");
    
    for (int i = 0; i < count; i++) {
        const char* name = (database[i].name[0] != '\0') ? database[i].name : "[empty]";
        const char* sname = (database[i].second_name[0] != '\0') ? database[i].second_name : "[empty]";
        const char* phone = (database[i].tel[0] != '\0') ? database[i].tel : "[empty]";
        
        printf("%-4d %-20s %-20s %-15s\n", 
               i + 1, 
               name, 
               sname, 
               phone);
    }
    printf("==================================================================\n");
}

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