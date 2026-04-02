#include <stdio.h>

int main() {
    int N;
    printf("Enter N: ");
    scanf("%d", &N);
    int matrix[N][N];

    for (int k = 0; k < N*N; k++) {
        int i = k/N;
        int j = k%N;
        if (i+j >= N-1) {
            matrix[i][j] = 1;
            } 
        else {
            matrix[i][j] = 0;
             }
        
        printf("%d\t", matrix[i][j]);
        if (j == N-1) printf("\n");
    }
    return 0;
}