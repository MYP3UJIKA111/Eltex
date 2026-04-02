#include <stdio.h>

int my_strlen(char* str); // Возвращает длину строки (количество символов до '\0')
char* my_strchr(char* str, char c); // Ищет символ c в строке str и возвращает указатель на него, или NULL
char* findSubstring(char* str, char* sub); // Функция поиска подстроки
void removeNewline(char* str); // Удаление символа новой строки 

int main() {
    char str[100];
    char sub[100];

    printf("Enter the main string: ");
    if (fgets(str, sizeof(str), stdin) == NULL) return 1;
    
    printf("Enter the substring to search: ");
    if (fgets(sub, sizeof(sub), stdin) == NULL) return 1;

    removeNewline(str);
    removeNewline(sub);

    char* result = findSubstring(str, sub);

    if (result != NULL) {

        int index = result - str; 
        
        printf("Substring found!\n");
        printf("Pointer to start: %p\n", (void*)result);
        printf("Position (index): %d\n", index);
        printf("Remaining string from this position: '%s'\n", result);
    } else {
        printf("Substring not found (returned NULL).\n");
    }

    return 0;
}

int my_strlen(char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

char* my_strchr(char* str, char c) {
    while (*str != '\0') {
        if (*str == c) {
            return str;
        }
        str++;
    }
    return NULL;
}

char* findSubstring(char* str, char* sub) {

    if (*sub == '\0') return str;

    for (char* pStr = str; *pStr != '\0'; pStr++) {
        char* p1 = pStr;
        char* p2 = sub;

        while (*p2 != '\0' && *p1 == *p2) {
            p1++;
            p2++;
        }

        if (*p2 == '\0') {
            return pStr;
        }
    }

    return NULL;
}

void removeNewline(char* str) {
    char* newline = my_strchr(str, '\n');
    if (newline != NULL) {
        *newline = '\0';
    }
}
