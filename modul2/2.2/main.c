#include "calculator.h"

void handleBinaryOperation(double (*operation)(double, double), const char* op_name) {
    double a, b;
    
    printf("Введите первое число: ");
    if (scanf("%lf", &a) != 1) {
        printf("Ошибка ввода!\n");
        clearInputBuffer();
        return;
    }
    
    printf("Введите второе число: ");
    if (scanf("%lf", &b) != 1) {
        printf("Ошибка ввода!\n");
        clearInputBuffer();
        return;
    }
    
    double result = operation(a, b);
    printf("%s: %.2lf %s %.2lf = %.2lf\n", op_name, a, op_name, b, result);
}

void handleUnaryOperation(double (*operation)(double), const char* op_name) {
    double a;
    
    printf("Введите число: ");
    if (scanf("%lf", &a) != 1) {
        printf("Ошибка ввода!\n");
        clearInputBuffer();
        return;
    }
    
    double result = operation(a);
    printf("%s(%.2lf) = %.2lf\n", op_name, a, result);
}

int main(void) {
    int choice;
    
    printf("========================================\n");
    printf("  ДОБРО ПОЖАЛОВАТЬ В КАЛЬКУЛЯТОР!      \n");
    printf("========================================\n");
    
    while (1) {
        showMenu();
        
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода! Введите число от 0 до 9.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
        
        switch (choice) {
            case 1:
                handleBinaryOperation(add, "Сложение");
                break;
            case 2:
                handleBinaryOperation(subtract, "Вычитание");
                break;
            case 3:
                handleBinaryOperation(multiply, "Умножение");
                break;
            case 4:
                handleBinaryOperation(divide, "Деление");
                break;
            case 5:
                handleBinaryOperation(power, "Возведение в степень");
                break;
            case 6:
                handleUnaryOperation(square_root, "Квадратный корень");
                break;
            case 7:
                handleUnaryOperation(sine, "Синус");
                break;
            case 8:
                handleUnaryOperation(cosine, "Косинус");
                break;
            case 9:
                handleUnaryOperation(tangent, "Тангенс");
                break;
            case 0:
                printf("\nВыход из программы. До свидания!\n");
                return 0;
            default:
                printf("Ошибка: Введите число от 0 до 9!\n");
                break;
        }
        
        printf("\nНажмите Enter для продолжения...");
        clearInputBuffer();
        getchar();
    }
    
    return 0;
}