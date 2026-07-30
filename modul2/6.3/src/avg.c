#include <stdarg.h>

double calculate(int num_args, ...) {
    if (num_args == 0) return 0.0;
    va_list args;
    va_start(args, num_args);
    double sum = 0.0;
    for (int i = 0; i < num_args; i++) {
        sum += va_arg(args, double);
    }
    va_end(args);
    return sum / num_args;
}

const char* get_name(void) {
    return "Среднее";
}

const char* get_symbol(void) {
    return "avg";
}