#include "udpchat.h"

static void print_usage(const char *prog) {
    printf("UDP Group Chat — равноправный чат через broadcast\n\n");
    printf("Использование:\n");
    printf("  %s              - запустить клиент чата\n", prog);
    printf("\nКоманды в чате:\n");
    printf("  /exit или exit  - покинуть чат\n");
    printf("  Ctrl+C          - покинуть чат (отправит LEAVE)\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        return 0;
    }

    return run_chat();
}