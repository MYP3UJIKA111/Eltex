#include <stdio.h>

int main() {
    int n;
    printf("Enter N: ");
    scanf("%d", &n);
    int matrix[n][n];
    
    int value = 1;
    int left = 0, right = n - 1;
    int top = 0, bottom = n - 1;
    
    while (left <= right && top <= bottom) {
        // Верхняя строка
        for (int col = left; col <= right; col++) {
            matrix[top][col] = value++;
        }
        top++;
        
        // Правый столбец
        for (int row = top; row <= bottom; row++) {
            matrix[row][right] = value++;
        }
        right--;
        
        // Нижняя строка
        if (top <= bottom) {
            for (int col = right; col >= left; col--) {
                matrix[bottom][col] = value++;
            }
            bottom--;
        }
        
        // Левый столбец
        if (left <= right) {
            for (int row = bottom; row >= top; row--) {
                matrix[row][left] = value++;
            }
            left++;
        }
    }
    
    // Вывод
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf(" %4d", matrix[i][j]);
        }
        printf("\n");
    }
    
    return 0;
}