#include "../include/tcpchat.h"

static void print_usage(const char *prog) {
    printf("TCP Chat - Client-Server Chat\n\n");
    printf("Usage:\n");
    printf("  %s server           - run as server\n", prog);
    printf("  %s client <IP>      - run as client (connect to server)\n", prog);
    printf("\nExamples:\n");
    printf("  %s server\n", prog);
    printf("  %s client 172.17.0.2\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "server") == 0) {
        return run_tcp_server();
    } else if (strcmp(argv[1], "client") == 0) {
        if (argc < 3) {
            printf("Error: client mode requires server IP\n");
            print_usage(argv[0]);
            return 1;
        }
        return run_tcp_client(argv[2]);
    } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    } else {
        printf("Unknown mode: %s\n", argv[1]);
        print_usage(argv[0]);
        return 1;
    }
}