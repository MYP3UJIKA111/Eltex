#include <stdio.h>

int main() {
    int num;
    int value;
    printf("Enter integer num: ");
    scanf("%d", &num);
    printf("Enter 3d byte for num (0-255): ");
    scanf("%d", &value);
    printf("result num before: \n");
    printf("in decimal %d\n", num);
    printf("in hex %x\n", num);
    unsigned char *ptr = (unsigned char *)&num; // получили указатель на первый байт числа
    *(ptr+2) = (unsigned char)value;// перезаписали 3 байт
    printf("result num after: \n");
    printf("in decimal %d\n", num);
    printf("in hex %x\n", num);
    return 0;
}