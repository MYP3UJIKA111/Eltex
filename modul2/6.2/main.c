#include "spravochnik.h"
#include <locale.h>

int main(void) {
    setlocale(LC_ALL, "ru_RU.UTF-8");

    head = NULL;
    count = 0;
    next_id = 1;

    int choice;

    while (1) {
        showMenu();
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода! Введите число от 1 до 7.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1: addAbonent(); break;
            case 2: deleteAbonent(); break;
            case 3: editAbonent(); break;
            case 4: searchByName(); break;
            case 5: searchById(); break;
            case 6: displayAll(); break;
            case 7:
                head = deleteList(head);
                count = 0;
                printf("\nВыход из программы. До свидания!\n");
                return 0;
            default:
                printf("Ошибка: Введите число от 1 до 7!\n");
        }
    }
    return 0;
}