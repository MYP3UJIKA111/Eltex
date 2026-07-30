#include "calculator.h"

double add(double a, double b) {
    return a + b;
}

double subtract(double a, double b) {
    return a - b;
}

double multiply(double a, double b) {
    return a * b;
}

double divide(double a, double b) {
    if (b == 0) {
        printf("Ошибка: Деление на ноль!\n");
        return NAN;
    }
    return a / b;
}

double power(double a, double b) {
    return pow(a, b);
}

double square_root(double a) {
    if (a < 0) {
        printf("Ошибка: Квадратный корень из отрицательного числа!\n");
        return 0;
    }
    return sqrt(a);
}

double sine(double a) {
    return sin(a);
}

double cosine(double a) {
    return cos(a);
}

double tangent(double a) {
    return tan(a);
}

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void showMenu(void) {
    printf("\n========================================\n");
    printf("         МАТЕМАТИЧЕСКИЙ КАЛЬКУЛЯТОР    \n");
    printf("========================================\n");
    printf(" 1) Сложение (+)                       \n");
    printf(" 2) Вычитание (-)                     \n");
    printf(" 3) Умножение (*)                     \n");
    printf(" 4) Деление (/)                       \n");
    printf(" 5) Возведение в степень (^)          \n");
    printf(" 6) Квадратный корень (√)             \n");
    printf(" 7) Синус (sin)                       \n");
    printf(" 8) Косинус (cos)                     \n");
    printf(" 9) Тангенс (tan)                     \n");
    printf(" 0) Выход                             \n");
    printf("========================================\n");
    printf("Выберите действие (0-9): ");
}