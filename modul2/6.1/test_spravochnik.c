#include "spravochnik.h"
#include <assert.h>

void test_add_and_find(void) {
    head = NULL;
    count = 0;
    next_id = 1;
    
    struct abonent ab1;
    memset(&ab1, 0, sizeof(ab1));
    strncpy(ab1.first_name, "Ivan", MAX_STR - 1);
    strncpy(ab1.second_name, "Petrov", MAX_STR - 1);
    
    int result = addAbonentDirect(&ab1);
    assert(result == 0);
    assert(count == 1);
    
    int found_id = findAbonentByName("Ivan");
    assert(found_id == 1);
    
    head = deleteList(head);
    count = 0;
    next_id = 1;
    
    printf("test_add_and_find: PASSED\n");
}

void test_delete(void) {
    head = NULL;
    count = 0;
    next_id = 1;
    
    struct abonent ab1, ab2;
    memset(&ab1, 0, sizeof(ab1));
    memset(&ab2, 0, sizeof(ab2));
    
    strncpy(ab1.first_name, "Ivan", MAX_STR - 1);
    strncpy(ab1.second_name, "Petrov", MAX_STR - 1);
    
    strncpy(ab2.first_name, "Petr", MAX_STR - 1);
    strncpy(ab2.second_name, "Ivanov", MAX_STR - 1);
    
    addAbonentDirect(&ab1);
    addAbonentDirect(&ab2);
    assert(count == 2);
    
    int result = deleteAbonentByIndex(0);
    assert(result == 0);
    assert(count == 1);
    
    head = deleteList(head);
    count = 0;
    next_id = 1;
    
    printf("test_delete: PASSED\n");
}

int main(void) {
    printf("Running tests...\n");
    test_add_and_find();
    test_delete();
    printf("All tests passed!\n");
    return 0;
}