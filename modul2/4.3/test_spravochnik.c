#include "spravochnik.h"
#include <assert.h>
#include <locale.h>

void reset_tree() {
    freeTree(root);
    root = NULL;
}

void test_avl_insert_and_balance() {
    printf("Тест: Вставка и балансировка АВЛ-дерева... ");
    reset_tree();
    
    struct abonent a1 = {0}; strcpy(a1.first_name, "A"); strcpy(a1.second_name, "A");
    struct abonent a2 = {0}; strcpy(a2.first_name, "B"); strcpy(a2.second_name, "B");
    struct abonent a3 = {0}; strcpy(a3.first_name, "C"); strcpy(a3.second_name, "C");
    
    root = insert(root, a1);
    root = insert(root, a2);
    root = insert(root, a3); // Должно вызвать левый поворот (LL)
    
    assert(root->data.second_name[0] == 'B'); // Корень после балансировки
    assert(root->left->data.second_name[0] == 'A');
    assert(root->right->data.second_name[0] == 'C');
    assert(getBalance(root) == 0);
    
    printf("ПРОЙДЕН\n");
}

void test_avl_search() {
    printf("Тест: Поиск в АВЛ-дереве... ");
    reset_tree();
    
    struct abonent a1 = {0}; strcpy(a1.first_name, "Иван"); strcpy(a1.second_name, "Иванов");
    struct abonent a2 = {0}; strcpy(a2.first_name, "Петр"); strcpy(a2.second_name, "Петров");
    
    root = insert(root, a1);
    root = insert(root, a2);
    
    assert(searchExact(root, "Иванов", "Иван") != NULL);
    assert(searchExact(root, "Петров", "Петр") != NULL);
    assert(searchExact(root, "Сидоров", "Алексей") == NULL);
    
    printf("ПРОЙДЕН\n");
}

void test_avl_delete() {
    printf("Тест: Удаление из АВЛ-дерева... ");
    reset_tree();
    
    struct abonent a1 = {0}; strcpy(a1.first_name, "A"); strcpy(a1.second_name, "A");
    struct abonent a2 = {0}; strcpy(a2.first_name, "B"); strcpy(a2.second_name, "B");
    struct abonent a3 = {0}; strcpy(a3.first_name, "C"); strcpy(a3.second_name, "C");
    
    root = insert(root, a1);
    root = insert(root, a2);
    root = insert(root, a3);
    
    root = deleteNode(root, "A", "A");
    assert(searchExact(root, "A", "A") == NULL);
    assert(searchExact(root, "B", "B") != NULL);
    assert(searchExact(root, "C", "C") != NULL);
    
    printf("ПРОЙДЕН\n");
}

void test_edit_rebalance() {
    printf("Тест: Редактирование ключа и ребалансировка... ");
    reset_tree();
    
    struct abonent a1 = {0}; strcpy(a1.first_name, "Иван"); strcpy(a1.second_name, "Абрамов");
    struct abonent a2 = {0}; strcpy(a2.first_name, "Петр"); strcpy(a2.second_name, "Борисов");
    
    root = insert(root, a1);
    root = insert(root, a2);
    
    // Меняем Абрамова на Яковлева (должен переместиться в конец дерева)
    struct TreeNode* node = searchExact(root, "Абрамов", "Иван");
    strcpy(node->data.second_name, "Яковлев");
    
    struct abonent updated = node->data;
    root = deleteNode(root, "Абрамов", "Иван");
    root = insert(root, updated);
    
    assert(searchExact(root, "Яковлев", "Иван") != NULL);
    assert(searchExact(root, "Абрамов", "Иван") == NULL);
    
    printf("ПРОЙДЕН\n");
}

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    printf("=====================================\n");
    printf("       ЗАПУСК ЮНИТ-ТЕСТОВ (АВЛ)      \n");
    printf("=====================================\n");
    
    test_avl_insert_and_balance();
    test_avl_search();
    test_avl_delete();
    test_edit_rebalance();
    
    printf("=====================================\n");
    printf(" ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО!\n");
    printf("=====================================\n");
    return 0;
}