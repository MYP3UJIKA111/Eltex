#include "../include/raw_capture.h"

static void print_usage(const char *prog) {
    printf("UDP RAW Socket Capture Tool\n\n");
    printf("Usage:\n");
    printf("  %s [filter]\n\n", prog);
    printf("Filters:\n");
    printf("  1 - UDP Chat (port %d)\n", CHAT_PORT);
    printf("  2 - DNS (port %d)\n", DNS_PORT);
    printf("  3 - All UDP packets\n");
    printf("\nExamples:\n");
    printf("  %s 1    # Capture UDP chat packets\n", prog);
    printf("  %s 2    # Capture DNS packets\n", prog);
    printf("  %s 3    # Capture all UDP packets\n", prog);
    printf("\nNote: Requires root privileges\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    int filter_num = atoi(argv[1]);
    filter_type_t filter;
    
    switch (filter_num) {
        case 1:
            filter = FILTER_CHAT;
            break;
        case 2:
            filter = FILTER_DNS;
            break;
        case 3:
            filter = FILTER_ALL;
            break;
        default:
            printf("Invalid filter: %d\n", filter_num);
            print_usage(argv[0]);
            return 1;
    }
    
    return run_capture(filter);
}