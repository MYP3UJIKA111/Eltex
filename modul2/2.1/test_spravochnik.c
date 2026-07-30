#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <locale.h> 
#include "spravochnik.h"

void reset_database() {
    count = 0;
    memset(database, 0, sizeof(database));
}

void test_validation() {
    printf("Тест: Валидация имени и телефона... ");
    assert(isValidName("Анастасия") == 1);
    assert(isValidName("Иван Петров") == 1);
    assert(isValidName("123") == 0);
    assert(isValidName("") == 0);
    assert(isValidPhone("+7 (961) 999-59-90") == 1);
    assert(isValidPhone("89619995990") == 1);
    assert(isValidPhone("abc") == 0);
    printf("ПРОЙДЕН\n");
}

void test_add_multiple_abonents() {
    printf("Тест: Добавление нескольких абонентов... ");
    reset_database();
    assert(count == 0);

    struct abonent a1 = {0};
    strcpy(a1.first_name, "Иван");
    strcpy(a1.second_name, "Иванов");
    strcpy(a1.phones[0], "+79001112233");
    strcpy(a1.emails[0], "ivan@test.ru");
    assert(addAbonentDirect(&a1) == 0);

    struct abonent a2 = {0};
    strcpy(a2.first_name, "Петр");
    strcpy(a2.second_name, "Петров");
    strcpy(a2.phones[0], "+79004445566");
    strcpy(a2.workplace, "Сбербанк");
    assert(addAbonentDirect(&a2) == 1);

    struct abonent a3 = {0};
    strcpy(a3.first_name, "Анна");
    strcpy(a3.second_name, "Сидорова");
    strcpy(a3.phones[0], "+79007778899");
    strcpy(a3.socials[0], "vk.com/anna");
    assert(addAbonentDirect(&a3) == 2);

    assert(count == 3);
    printf("ПРОЙДЕН\n");
}

void test_search_multiple() {
    printf("Тест: Поиск среди нескольких абонентов... ");
    reset_database();
    
    struct abonent temp = {0};
    strcpy(temp.first_name, "Дмитрий"); strcpy(temp.second_name, "Достовалов");
    addAbonentDirect(&temp);
    
    strcpy(temp.first_name, "Алексей"); strcpy(temp.second_name, "Смирнов");
    addAbonentDirect(&temp);

    assert(findAbonentByName("Дмитрий") == 0);
    assert(findAbonentByName("смирнов") == 1);
    assert(findAbonentByName("Никто") == -1);
    
    printf("ПРОЙДЕН\n");
}

void test_edit_abonent() {
    printf("Тест: Редактирование данных абонента... ");
    reset_database();
    
    struct abonent a1 = {0};
    strcpy(a1.first_name, "Тест");
    strcpy(a1.second_name, "Тестов");
    strcpy(a1.phones[0], "111111");
    addAbonentDirect(&a1);

    int idx = findAbonentByName("Тест");
    assert(idx == 0);
    
    strcpy(database[idx].phones[0], "999999");
    strcpy(database[idx].position, "Старший инженер");

    assert(strcmp(database[0].phones[0], "999999") == 0);
    assert(strcmp(database[0].position, "Старший инженер") == 0);
    assert(strcmp(database[0].first_name, "Тест") == 0);
    
    printf("ПРОЙДЕН\n");
}

void test_delete_and_shift() {
    printf("Тест: Удаление и сдвиг массива... ");
    reset_database();
    
    struct abonent a1 = {0}; strcpy(a1.first_name, "Иван"); strcpy(a1.second_name, "Иванов");
    addAbonentDirect(&a1);
    
    struct abonent a2 = {0}; strcpy(a2.first_name, "Петр"); strcpy(a2.second_name, "Петров");
    addAbonentDirect(&a2);
    
    struct abonent a3 = {0}; strcpy(a3.first_name, "Анна"); strcpy(a3.second_name, "Сидорова");
    addAbonentDirect(&a3);
    
    assert(count == 3);

    assert(deleteAbonentByIndex(0) == 0);
    assert(count == 2);
    assert(strcmp(database[0].first_name, "Петр") == 0);
    assert(strcmp(database[0].second_name, "Петров") == 0);
    assert(strcmp(database[1].first_name, "Анна") == 0);

    assert(deleteAbonentByIndex(10) == -1);
    assert(count == 2);
    
    printf("ПРОЙДЕН\n");
}

void test_add_invalid_data() {
    printf("Тест: Отклонение невалидных данных... ");
    reset_database();
    
    struct abonent bad = {0};
    strcpy(bad.first_name, "12345");
    strcpy(bad.second_name, "Иванов");
    
    assert(addAbonentDirect(&bad) == -1);
    assert(count == 0);
    
    printf("ПРОЙДЕН\n");
}

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    
    printf("=====================================\n");
    printf("       ЗАПУСК ЮНИТ-ТЕСТОВ             \n");
    printf("=====================================\n");
    
    test_validation();
    test_add_multiple_abonents();
    test_search_multiple();
    test_edit_abonent();
    test_delete_and_shift();
    test_add_invalid_data();
    
    printf("=====================================\n");
    printf(" ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО!\n");
    printf("=====================================\n");
    
    return 0;
}