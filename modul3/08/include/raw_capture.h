#ifndef RAW_CAPTURE_H
#define RAW_CAPTURE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netpacket/packet.h>

#define MAX_PACKET_SIZE 65536
#define MAX_CAPTURED 1000
#define CHAT_PORT 50000
#define DNS_PORT 53

// Структура для хранения захваченного пакета
typedef struct {
    struct timeval timestamp;
    unsigned char src_mac[6];
    unsigned char dst_mac[6];
    char src_ip[INET_ADDRSTRLEN];
    char dst_ip[INET_ADDRSTRLEN];
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t udp_length;
    unsigned char payload[1024];
    int payload_len;
} captured_packet_t;

// Типы фильтров
typedef enum {
    FILTER_CHAT = 1,    // Фильтр для UDP чата (порт 50000)
    FILTER_DNS,         // Фильтр для DNS (порт 53)
    FILTER_ALL          // Все UDP пакеты
} filter_type_t;

// Прототипы функций
int run_capture(filter_type_t filter);
void print_packet(const captured_packet_t *pkt, int index);
void save_to_file(const captured_packet_t *packets, int count, const char *filename);

#endif /* RAW_CAPTURE_H */