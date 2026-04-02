
#include <stdio.h>

int main() {

    int input_num;
    int current_bit;
    int count_ones = 0;
    unsigned char new_byte_value;

    printf("Enter an integer: ");
    scanf("%d", &input_num);

    printf("Binary representation: ");
    for(int i = sizeof(int)*8-1; i>=0; --i) {
        current_bit = (input_num >> i) & 1;
        printf("%d", current_bit);

        if (current_bit == 1) count_ones++;
    }
    printf("\n");
    printf("Number of units: %d", count_ones);

    // Ввод нового значения для третьего байта
    printf("\nEnter a new value for the third byte (0-255): ");
    scanf("%hhu", &new_byte_value);

    // Замена третьего байта
    int modified_num = input_num;
    modified_num = (modified_num & 0xFF00FFFF) | (new_byte_value << 16);

    // Вывод результата
    printf("\nAfter replacing the third byte: \n");
    printf("New number:%d\n", modified_num);
    printf("In hexadecimal: 0x%08X\n", modified_num);
    
    // Счетчик единиц для нового числа
    count_ones = 0;
    printf("Binary representation:");
    for(int i = sizeof(int)*8-1; i>=0; --i) {
        current_bit = (modified_num >> i) & 1;
        printf("%d", current_bit);
        if(current_bit == 1) count_ones++;
    }
    printf("\nNumber of units: %d\n", count_ones);

    return 0;
}
