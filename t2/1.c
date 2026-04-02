#include <stdio.h>

int main() {
    int n;
    printf("Enter N: ");
    scanf("%d", &n);
    int matrix[n][n];
    // Заполнение матрицы
    for (int k = 0; k < n*n; k++) {
        int i = k/n; // номер строки
        int j = k%n; // номер столбца
        matrix[i][j] = k + 1;
        printf("%d\t", matrix[i][j]);
        if (j == n-1) printf("\n");
    }
    return 0;
}

