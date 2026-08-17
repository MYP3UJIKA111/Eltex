#ifndef RAW_UDP_COMMON_H
#define RAW_UDP_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdint.h>

#define SERVER_PORT 51000
#define BUFFER_SIZE 1024

// Псевдозаголовок для UDP checksum (IPv4)
struct pseudo_header {
    uint32_t source_addr;
    uint32_t dest_addr;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t udp_length;
};

// Функция для подсчёта checksum
uint16_t checksum(uint16_t *data, int len);

#endif