#include <stdarg.h>

double calculate(int num_args, ...) {
    va_list args;
    va_start(args, num_args);
    if (num_args == 0) { va_end(args); return 0.0; }
    double min = va_arg(args, double);
    for (int i = 1; i < num_args; i++) {
        double current = va_arg(args, double);
        if (current < min) min = current;
    }
    va_end(args);
    return min;
}

const char* get_name(void) {
    return "Минимум";
}

const char* get_symbol(void) {
    return "min";
}