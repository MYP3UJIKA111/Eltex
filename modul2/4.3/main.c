#include "spravochnik.h"
#include <locale.h>

int getValidMenuChoice() {
    char buffer[32];
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) continue;
        int choice;
        if (sscanf(buffer, "%d", &choice) == 1 && choice >= 1 && choice <= 7) {
            return choice;
        }
        printf("Ошибка ввода! Введите число от 1 до 7.\n");
        printf("Выберите пункт меню (1-7): ");
    }
}

int main(void) {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    root = NULL;
    next_id = 1;

    while (1) {
        showMenu();
        int choice = getValidMenuChoice();

        switch (choice) {
            case 1: addAbonent(); break;
            case 2: deleteAbonent(); break;
            case 3: editAbonent(); break;
            case 4: searchMenu(); break;
            case 5: displayAll(); break;
            case 6: displayAsTree(); break;
            case 7:
                freeTree(root);
                printf("\nВыход из программы. До свидания!\n");
                return 0;
            default:
                printf("Ошибка: Введите число от 1 до 7!\n");
        }
    }
    return 0;
}