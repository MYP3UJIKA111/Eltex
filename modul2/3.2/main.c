#include "ip_simulator.h"

int main(int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, "Ошибка: Неверное количество аргументов!\n");
        printHelp(argv[0]);
        return 1;
    }
    
    const char* gateway_str = argv[1];
    const char* mask_str = argv[2];
    int N = atoi(argv[3]);
    
    if (N <= 0) {
        fprintf(stderr, "Ошибка: N должно быть положительным числом!\n");
        return 1;
    }
    
    if (!isValidIP(gateway_str)) {
        fprintf(stderr, "Ошибка: Неверный IP-адрес шлюза '%s'\n", gateway_str);
        return 1;
    }
    
    if (!isValidMask(mask_str)) {
        fprintf(stderr, "Ошибка: Неверная маска подсети '%s'\n", mask_str);
        return 1;
    }
    
    uint32_t gateway_ip = ipStrToUint32(gateway_str);
    uint32_t subnet_mask = parseMask(mask_str);
    
    char gateway_str2[MAX_IP_STR];
    char network_str[MAX_IP_STR];
    ipUint32ToStr(gateway_ip, gateway_str2, sizeof(gateway_str2));
    ipUint32ToStr(gateway_ip & subnet_mask, network_str, sizeof(network_str));
    
    printf("========================================\n");
    printf("  ИМИТАЦИЯ МАРШРУТИЗАЦИИ IPv4\n");
    printf("========================================\n");
    printf("Шлюз:        %s\n", gateway_str2);
    printf("Маска:       %s\n", mask_str);
    printf("Сеть:        %s\n", network_str);
    printf("Пакетов:     %d\n", N);
    printf("========================================\n");
    
    srand((unsigned int)time(NULL));
    
    PacketInfo* packets = (PacketInfo*)malloc(N * sizeof(PacketInfo));
    if (packets == NULL) {
        fprintf(stderr, "Ошибка: Не удалось выделить память!\n");
        return 1;
    }
    
    PacketStatistics stats = {0};
    stats.total_packets = N;
    
    printf("\n--- ГЕНЕРАЦИЯ И ОБРАБОТКА ПАКЕТОВ ---\n");
    printf(" №   | IP-адрес назначения | Результат\n");
    printf("-----|---------------------|-----------\n");
    
    int print_limit = (N <= 20) ? N : 20;
    
    for (int i = 0; i < N; i++) {
        uint32_t dest_ip = generateRandomIP();
        
        packets[i].ip_addr = dest_ip;
        ipUint32ToStr(dest_ip, packets[i].ip_str, sizeof(packets[i].ip_str));
        packets[i].is_local = isInSameSubnet(dest_ip, gateway_ip, subnet_mask);
        
        if (packets[i].is_local) {
            stats.local_packets++;
        } else {
            stats.remote_packets++;
        }
        
        if (i < print_limit) {
            printPacketInfo(&packets[i], i + 1);
        }
    }
    
    if (N > 20) {
        printf("... и еще %d пакетов обработано (скрыто для экономии места).\n", N - 20);
    }
    
    printStatistics(&stats);
    
    free(packets);
    return 0;
}