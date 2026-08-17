#define _DEFAULT_SOURCE
#include "../include/raw_capture.h"

static volatile sig_atomic_t g_stop = 0;

static void signal_handler(int sig) {
    (void)sig;
    g_stop = 1;
}

// Получение индекса сетевого интерфейса
int get_interface_index(const char *ifname) {
    int sockfd;
    struct ifreq ifr;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return -1;
    }
    
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        close(sockfd);
        return -1;
    }
    
    close(sockfd);
    return ifr.ifr_ifindex;
}

// Парсинг Ethernet заголовка
int parse_ethernet(const unsigned char *buffer, struct ethhdr *eth) {
    memcpy(eth->h_dest, buffer, ETH_ALEN);
    memcpy(eth->h_source, buffer + ETH_ALEN, ETH_ALEN);
    memcpy(&eth->h_proto, buffer + 2 * ETH_ALEN, sizeof(eth->h_proto));
    return ETH_HLEN;
}

// Парсинг IP заголовка
int parse_ip(const unsigned char *buffer, struct iphdr *ip) {
    memcpy(ip, buffer, sizeof(struct iphdr));
    return ip->ihl * 4;  // Длина IP заголовка в байтах
}

// Парсинг UDP заголовка
int parse_udp(const unsigned char *buffer, struct udphdr *udp) {
    memcpy(udp, buffer, sizeof(struct udphdr));
    return sizeof(struct udphdr);
}

// Применение фильтра
int apply_filter(const captured_packet_t *pkt, filter_type_t filter) {
    switch (filter) {
        case FILTER_CHAT:
            // Фильтр для UDP чата: порт 50000
            return (ntohs(pkt->src_port) == CHAT_PORT || ntohs(pkt->dst_port) == CHAT_PORT);
        
        case FILTER_DNS:
            // Фильтр для DNS: порт 53
            return (ntohs(pkt->src_port) == DNS_PORT || ntohs(pkt->dst_port) == DNS_PORT);
        
        case FILTER_ALL:
            // Все UDP пакеты
            return 1;
        
        default:
            return 0;
    }
}

// Вывод пакета на экран
void print_packet(const captured_packet_t *pkt, int index) {
    printf("\n=== Packet #%d ===\n", index);
    printf("Time: %.6f seconds from capture start\n", 
           pkt->timestamp.tv_sec + pkt->timestamp.tv_usec / 1000000.0);
    
    printf("MAC Addresses:\n");
    printf("  Source: %02x:%02x:%02x:%02x:%02x:%02x\n",
           pkt->src_mac[0], pkt->src_mac[1], pkt->src_mac[2],
           pkt->src_mac[3], pkt->src_mac[4], pkt->src_mac[5]);
    printf("  Destination: %02x:%02x:%02x:%02x:%02x:%02x\n",
           pkt->dst_mac[0], pkt->dst_mac[1], pkt->dst_mac[2],
           pkt->dst_mac[3], pkt->dst_mac[4], pkt->dst_mac[5]);
    
    printf("IP Addresses:\n");
    printf("  Source: %s\n", pkt->src_ip);
    printf("  Destination: %s\n", pkt->dst_ip);
    
    printf("UDP Ports:\n");
    printf("  Source: %d\n", ntohs(pkt->src_port));
    printf("  Destination: %d\n", ntohs(pkt->dst_port));
    printf("  Length: %d bytes\n", ntohs(pkt->udp_length));
    
    if (pkt->payload_len > 0) {
        printf("Payload (%d bytes):\n", pkt->payload_len);
        printf("  ");
        for (int i = 0; i < pkt->payload_len && i < 64; i++) {
            printf("%02x ", pkt->payload[i]);
            if ((i + 1) % 16 == 0) printf("\n  ");
        }
        printf("\n");
        
        // Попытка вывести как текст
        printf("  Text: ");
        for (int i = 0; i < pkt->payload_len && i < 64; i++) {
            if (pkt->payload[i] >= 32 && pkt->payload[i] < 127) {
                printf("%c", pkt->payload[i]);
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
}

// Сохранение в файл
void save_to_file(const captured_packet_t *packets, int count, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return;
    }
    
    fprintf(fp, "UDP Packet Capture Report\n");
    fprintf(fp, "Total packets: %d\n\n", count);
    
    for (int i = 0; i < count; i++) {
        const captured_packet_t *pkt = &packets[i];
        fprintf(fp, "=== Packet #%d ===\n", i + 1);
        fprintf(fp, "Time: %.6f seconds from capture start\n",
                pkt->timestamp.tv_sec + pkt->timestamp.tv_usec / 1000000.0);
        
        fprintf(fp, "MAC Addresses:\n");
        fprintf(fp, "  Source: %02x:%02x:%02x:%02x:%02x:%02x\n",
                pkt->src_mac[0], pkt->src_mac[1], pkt->src_mac[2],
                pkt->src_mac[3], pkt->src_mac[4], pkt->src_mac[5]);
        fprintf(fp, "  Destination: %02x:%02x:%02x:%02x:%02x:%02x\n",
                pkt->dst_mac[0], pkt->dst_mac[1], pkt->dst_mac[2],
                pkt->dst_mac[3], pkt->dst_mac[4], pkt->dst_mac[5]);
        
        fprintf(fp, "IP Addresses:\n");
        fprintf(fp, "  Source: %s\n", pkt->src_ip);
        fprintf(fp, "  Destination: %s\n", pkt->dst_ip);
        
        fprintf(fp, "UDP Ports:\n");
        fprintf(fp, "  Source: %d\n", ntohs(pkt->src_port));
        fprintf(fp, "  Destination: %d\n", ntohs(pkt->dst_port));
        fprintf(fp, "  Length: %d bytes\n", ntohs(pkt->udp_length));
        
        if (pkt->payload_len > 0) {
            fprintf(fp, "Payload (%d bytes):\n", pkt->payload_len);
            fprintf(fp, "  ");
            for (int j = 0; j < pkt->payload_len && j < 64; j++) {
                fprintf(fp, "%02x ", pkt->payload[j]);
                if ((j + 1) % 16 == 0) fprintf(fp, "\n  ");
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\n");
    }
    
    fclose(fp);
    printf("Data saved to %s\n", filename);
}

int run_capture(filter_type_t filter) {
    int raw_fd;
    unsigned char buffer[MAX_PACKET_SIZE];
    captured_packet_t packets[MAX_CAPTURED];
    int packet_count = 0;
    struct timeval start_time;
    struct ethhdr eth;
    struct iphdr ip;
    struct udphdr udp;
    
    // Установка обработчика сигналов
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    // Создание RAW сокета на уровне канала
    raw_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_fd < 0) {
        perror("socket");
        return -1;
    }
    
    // Привязка к интерфейсу docker0
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    
    // Пробуем привязаться к docker0
    int ifindex = get_interface_index("any");
    if (ifindex < 0) {
        printf("Warning: Could not find docker0 interface, using all interfaces\n");
        sll.sll_ifindex = 0;  // Все интерфейсы
    } else {
        sll.sll_ifindex = ifindex;
        printf("Binding to interface: docker0 (index=%d)\n", ifindex);
    }
    
    if (bind(raw_fd, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        perror("bind");
        close(raw_fd);
        return -1;
    }
    
    printf("=== UDP RAW Socket Capture ===\n");
    printf("Filter: ");
    switch (filter) {
        case FILTER_CHAT: printf("UDP Chat (port %d)\n", CHAT_PORT); break;
        case FILTER_DNS: printf("DNS (port %d)\n", DNS_PORT); break;
        case FILTER_ALL: printf("All UDP packets\n"); break;
    }
    printf("Capturing... Press Ctrl+C to stop\n\n");
    
    gettimeofday(&start_time, NULL);
    
    while (!g_stop && packet_count < MAX_CAPTURED) {
        ssize_t n = recvfrom(raw_fd, buffer, sizeof(buffer), 0, NULL, NULL);
        
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("recvfrom");
            break;
        }
        
        // Проверяем минимальный размер (Ethernet + IP + UDP)
        if (n < (ssize_t)(ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr))) {
            continue;
        }
        
        // Парсинг Ethernet заголовка
        int offset = parse_ethernet(buffer, &eth);
        
        // Проверяем, что это IP пакет
        if (ntohs(eth.h_proto) != ETH_P_IP) continue;
        
        // Проверяем размер IP заголовка
        if (offset + sizeof(struct iphdr) > n) continue;
        
        // Парсинг IP заголовка
        offset += parse_ip(buffer + offset, &ip);
        
        // Проверяем, что это UDP пакет
        if (ip.protocol != IPPROTO_UDP) continue;
        
        // Проверяем размер
        if (offset + sizeof(struct udphdr) > n) continue;
        
        // Парсинг UDP заголовка
        offset += parse_udp(buffer + offset, &udp);
        
        // Создание структуры пакета
        captured_packet_t pkt;
        gettimeofday(&pkt.timestamp, NULL);
        
        // Вычисляем время от начала захвата
        pkt.timestamp.tv_sec -= start_time.tv_sec;
        pkt.timestamp.tv_usec -= start_time.tv_usec;
        if (pkt.timestamp.tv_usec < 0) {
            pkt.timestamp.tv_sec--;
            pkt.timestamp.tv_usec += 1000000;
        }
        
        // MAC адреса
        memcpy(pkt.src_mac, eth.h_source, ETH_ALEN);
        memcpy(pkt.dst_mac, eth.h_dest, ETH_ALEN);
        
        // IP адреса
        inet_ntop(AF_INET, &ip.saddr, pkt.src_ip, sizeof(pkt.src_ip));
        inet_ntop(AF_INET, &ip.daddr, pkt.dst_ip, sizeof(pkt.dst_ip));
        
        // UDP порты
        pkt.src_port = udp.source;
        pkt.dst_port = udp.dest;
        pkt.udp_length = udp.len;
        
        // Payload
        int payload_len = ntohs(udp.len) - sizeof(struct udphdr);
        if (payload_len > 0 && payload_len <= (int)sizeof(pkt.payload) && offset + payload_len <= n) {
            memcpy(pkt.payload, buffer + offset, payload_len);
            pkt.payload_len = payload_len;
        } else {
            pkt.payload_len = 0;
        }
        
        // Применение фильтра
        if (apply_filter(&pkt, filter)) {
            packets[packet_count++] = pkt;
            printf(".");
            fflush(stdout);
        }
    }
    
    close(raw_fd);
    
    printf("\n\nCapture stopped. Total packets captured: %d\n\n", packet_count);
    
    // Вывод всех пакетов
    for (int i = 0; i < packet_count; i++) {
        print_packet(&packets[i], i + 1);
    }
    
    // Сохранение в файл
    if (packet_count > 0) {
        save_to_file(packets, packet_count, "capture_report.txt");
    }
    
    return 0;
}