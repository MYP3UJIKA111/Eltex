#ifndef IP_SIMULATOR_H
#define IP_SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <arpa/inet.h>

#define MAX_IP_STR 16

typedef struct {
    int total_packets;
    int local_packets;
    int remote_packets;
} PacketStatistics;

typedef struct {
    uint32_t ip_addr;
    char ip_str[MAX_IP_STR];
    int is_local;
} PacketInfo;

uint32_t ipStrToUint32(const char* ip_str);
void ipUint32ToStr(uint32_t ip, char* buffer, size_t size);
uint32_t generateRandomIP(void);
int isInSameSubnet(uint32_t ip, uint32_t gateway, uint32_t mask);
uint32_t parseMask(const char* mask_str);
void printStatistics(const PacketStatistics* stats);
void printPacketInfo(const PacketInfo* packet, int index);
int isValidIP(const char* ip_str);
int isValidMask(const char* mask_str);
void printHelp(const char* prog_name);

#endif