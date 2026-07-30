#include "spravochnik.h"
#include <locale.h>
#include <string.h>

int main(void) {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    memset(database, 0, sizeof(database));
    int choice;

    while (1) {
        showMenu();
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода! Введите число от 1 до 6.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1: addAbonent(); break;
            case 2: deleteAbonent(); break;
            case 3: editAbonent(); break;
            case 4: searchByName(); break;
            case 5: displayAll(); break;
            case 6:
                printf("\nВыход из программы. До свидания!\n");
                return 0;
            default:
                printf("Ошибка: Введите число от 1 до 6!\n");
        }
    }
    return 0;
}