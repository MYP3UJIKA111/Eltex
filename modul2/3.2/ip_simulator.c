#include "ip_simulator.h"

// Преобразование строки IP в 32-битное число (порядок байт хоста)
uint32_t ipStrToUint32(const char* ip_str) {
    struct in_addr addr;
    if (inet_pton(AF_INET, ip_str, &addr) != 1) {
        return 0;
    }
    return ntohl(addr.s_addr); // Сетевой порядок - порядок хоста
}

// Преобразование 32-битного числа обратно в строку IP
void ipUint32ToStr(uint32_t ip, char* buffer, size_t size) {
    struct in_addr addr;
    addr.s_addr = htonl(ip); // Порядок хоста - сетевой порядок
    inet_ntop(AF_INET, &addr, buffer, size);
}

// Генерация случайного IP-адреса из 4 случайных октетов
uint32_t generateRandomIP(void) {
    uint32_t ip = 0;
    for (int i = 0; i < 4; i++) {
        uint32_t octet = rand() % 256;
        ip = (ip << 8) | octet; // Сдвиг на 8 бит и добавление октета
    }
    return ip;
}

// Проверка принадлежности IP к подсети через побитовое И с маской
int isInSameSubnet(uint32_t ip, uint32_t gateway, uint32_t mask) {
    uint32_t ip_network = ip & mask;
    uint32_t gateway_network = gateway & mask;
    return (ip_network == gateway_network);
}

// Парсинг маски
uint32_t parseMask(const char* mask_str) {
    if (strchr(mask_str, '.') != NULL) {
        // Формат с точками
        struct in_addr addr;
        if (inet_pton(AF_INET, mask_str, &addr) == 1) {
            return ntohl(addr.s_addr);
        }
    } else {
        // Числовой формат
        int prefix = atoi(mask_str);
        if (prefix >= 0 && prefix <= 32) {
            return (prefix == 0) ? 0 : (~0U << (32 - prefix));
        }
    }
    return 0;
}

// Проверка корректности IP-адреса
int isValidIP(const char* ip_str) {
    struct in_addr addr;
    return (inet_pton(AF_INET, ip_str, &addr) == 1);
}

// Проверка корректности маски (все единицы идут подряд)
int isValidMask(const char* mask_str) {
    uint32_t mask = parseMask(mask_str);
    if (mask == 0 && strcmp(mask_str, "0") != 0 && strcmp(mask_str, "0.0.0.0") != 0) {
        return 0;
    }
    
    int found_zero = 0;
    for (int i = 31; i >= 0; i--) {
        int bit = (mask >> i) & 1; // Извлекаем i-й бит
        if (bit == 0) {
            found_zero = 1;
        } else if (found_zero) {
            return 0; // Единица после нуля - невалидная маска
        }
    }
    return 1;
}

// Вывод итоговой статистики обработки пакетов
void printStatistics(const PacketStatistics* stats) {
    float local_pct = (float)stats->local_packets / stats->total_packets * 100.0f;
    float remote_pct = (float)stats->remote_packets / stats->total_packets * 100.0f;

    printf("\n========================================\n");
    printf("        СТАТИСТИКА ОБРАБОТКИ\n");
    printf("========================================\n");
    printf("Всего пакетов:        %d\n", stats->total_packets);
    printf("----------------------------------------\n");
    printf("В своей подсети:      %d (%.1f%%)\n", stats->local_packets, local_pct);
    printf("В других сетях:       %d (%.1f%%)\n", stats->remote_packets, remote_pct);
    printf("========================================\n");
}

// Вывод информации об одном пакете
void printPacketInfo(const PacketInfo* packet, int index) {
    const char* local_str = packet->is_local ? "СВОЯ СЕТЬ" : "ДРУГАЯ СЕТЬ";
    printf("#%-3d | %-15s | %s\n", index, packet->ip_str, local_str);
}

// Вывод справки по использованию программы
void printHelp(const char* prog_name) {
    printf("\n========================================\n");
    printf("  ИМИТАЦИЯ МАРШРУТИЗАЦИИ IPv4\n");
    printf("========================================\n");
    printf("Использование: %s <IP шлюза> <маска> <N>\n", prog_name);
    printf("\nПараметры:\n");
    printf("  IP шлюза  - IP-адрес шлюза (например, 192.168.1.1)\n");
    printf("  маска     - маска подсети (например, 255.255.255.0 или 24)\n");
    printf("  N         - количество пакетов для генерации\n");
    printf("\nПример:\n");
    printf("  %s 192.168.1.1 255.255.255.0 10\n", prog_name);
    printf("  %s 192.168.1.1 24 10\n", prog_name);
    printf("========================================\n");
}