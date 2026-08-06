#include "prodcons.h"

static void print_usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s -p [max_blocks]   - run producer\n", prog);
    printf("  %s -c                - run consumer\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-p") == 0) {
        int max_blocks = 0;
        if (argc >= 3) {
            max_blocks = atoi(argv[2]);
            if (max_blocks <= 0) max_blocks = 0;
        }
        return run_producer(max_blocks);
    }

    if (strcmp(argv[1], "-c") == 0) {
        return run_consumer();
    }

    print_usage(argv[0]);
    return 1;
}