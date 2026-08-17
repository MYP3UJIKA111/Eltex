#include "raw_udp_common.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

typedef struct client_state {
    struct sockaddr_in addr;
    uint32_t counter;
    int active;
} client_state_t;

#define MAX_CLIENTS 64
client_state_t clients[MAX_CLIENTS];
int server_sock = -1;

client_state_t *find_client(struct sockaddr_in *addr) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active &&
            clients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
            clients[i].addr.sin_port == addr->sin_port) {
            return &clients[i];
        }
    }
    return NULL;
}

client_state_t *find_or_create_client(struct sockaddr_in *addr) {
    client_state_t *client = find_client(addr);
    if (client) {
        return client;
    }
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            clients[i].addr = *addr;
            clients[i].counter = 0;
            clients[i].active = 1;
            printf("New client %s:%d connected\n",
                   inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
            return &clients[i];
        }
    }
    return NULL;
}

void remove_client(struct sockaddr_in *addr) {
    client_state_t *client = find_client(addr);
    if (client) {
        client->active = 0;
        printf("Client %s:%d disconnected\n",
               inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
    }
}

void signal_handler(int sig) {
    printf("\nServer shutting down...\n");
    if (server_sock != -1) close(server_sock);
    exit(0);
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    unsigned char buffer[BUFFER_SIZE];
    int ret;

    memset(clients, 0, sizeof(clients));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    server_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (server_sock < 0) {
        perror("socket");
        exit(1);
    }

    printf("Raw UDP server started on port %d\n", SERVER_PORT);
    printf("Waiting for messages...\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        ret = recvfrom(server_sock, buffer, BUFFER_SIZE, 0,
                       (struct sockaddr *)&client_addr, &client_len);
        if (ret < 0) {
            perror("recvfrom");
            continue;
        }

        struct iphdr *ip_hdr = (struct iphdr *)buffer;
        int ip_header_len = ip_hdr->ihl * 4;

        if (ip_hdr->protocol != IPPROTO_UDP) {
            continue;
        }

        struct udphdr *udp_hdr = (struct udphdr *)(buffer + ip_header_len);
        uint16_t src_port = ntohs(udp_hdr->source);
        uint16_t dst_port = ntohs(udp_hdr->dest);

        if (dst_port != SERVER_PORT) {
            continue;
        }

        client_addr.sin_port = udp_hdr->source;

        unsigned char *data = buffer + ip_header_len + sizeof(struct udphdr);
        int data_len = ret - ip_header_len - sizeof(struct udphdr);

        if (data_len == 5 && strncmp((char *)data, "CLOSE", 5) == 0) {
            remove_client(&client_addr);
            continue;
        }

        client_state_t *client = find_or_create_client(&client_addr);
        if (!client) {
            fprintf(stderr, "Too many clients\n");
            continue;
        }
        client->counter++;

        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "%.*s %u",
                 data_len, (char *)data, client->counter);

        printf("Received from %s:%d: '%.*s', reply: '%s' (counter: %u)\n",
               inet_ntoa(client_addr.sin_addr), src_port,
               data_len, data, response, client->counter);

        struct pseudo_header pseudo;
        pseudo.source_addr = client_addr.sin_addr.s_addr;
        pseudo.dest_addr = server_addr.sin_addr.s_addr;
        pseudo.placeholder = 0;
        pseudo.protocol = IPPROTO_UDP;
        pseudo.udp_length = htons(sizeof(struct udphdr) + strlen(response));

        unsigned char out_buffer[BUFFER_SIZE];
        struct udphdr *out_udp = (struct udphdr *)out_buffer;
        out_udp->source = htons(SERVER_PORT);
        out_udp->dest = udp_hdr->source;
        out_udp->len = htons(sizeof(struct udphdr) + strlen(response));
        out_udp->check = 0;

        memcpy(out_buffer + sizeof(struct udphdr), response, strlen(response));

        unsigned char checksum_buf[BUFFER_SIZE];
        memcpy(checksum_buf, &pseudo, sizeof(pseudo));
        memcpy(checksum_buf + sizeof(pseudo), out_buffer,
               sizeof(struct udphdr) + strlen(response));
        out_udp->check = checksum((uint16_t *)checksum_buf,
                                  sizeof(pseudo) + sizeof(struct udphdr) + strlen(response));

        struct sockaddr_in dest_addr = client_addr;
        ret = sendto(server_sock, out_buffer,
                     sizeof(struct udphdr) + strlen(response),
                     0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (ret < 0) {
            perror("sendto");
        }
    }

    close(server_sock);
    return 0;
}