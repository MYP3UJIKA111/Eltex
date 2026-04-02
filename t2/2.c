#include <stdio.h>

int main() {
    int N;
    printf("Enter N: ");
    scanf("%d", &N);
    int arr[N];

    printf("Enter %d array elements: ", N);
    for(int i = 0; i < N; i++){
        scanf("%d", &arr[i]);
    }

    printf("\n");
    for (int i = N-1; i >= 0; i--){
        printf("%d ", arr[i]);
    }

    return 0;
}