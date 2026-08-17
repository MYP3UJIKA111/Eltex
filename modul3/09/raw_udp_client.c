#include "raw_udp_common.h"
#include <signal.h>
#include <poll.h>

int raw_sock = -1;
int udp_sock = -1;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
int server_port = SERVER_PORT;
int client_port = 0;

void signal_handler(int sig) {
    (void)sig;
    printf("\nClient shutting down, sending CLOSE...\n");
    
    if (raw_sock != -1) {
        unsigned char out_buffer[BUFFER_SIZE];
        struct udphdr *udp_hdr = (struct udphdr *)out_buffer;
        udp_hdr->source = htons(client_port);
        udp_hdr->dest = htons(server_port);
        udp_hdr->len = htons(sizeof(struct udphdr) + 5);
        udp_hdr->check = 0;
        
        memcpy(out_buffer + sizeof(struct udphdr), "CLOSE", 5);
        
        struct pseudo_header pseudo;
        pseudo.source_addr = client_addr.sin_addr.s_addr;
        pseudo.dest_addr = server_addr.sin_addr.s_addr;
        pseudo.placeholder = 0;
        pseudo.protocol = IPPROTO_UDP;
        pseudo.udp_length = udp_hdr->len;
        
        unsigned char checksum_buf[BUFFER_SIZE];
        memcpy(checksum_buf, &pseudo, sizeof(pseudo));
        memcpy(checksum_buf + sizeof(pseudo), out_buffer,
               sizeof(struct udphdr) + 5);
        udp_hdr->check = checksum((uint16_t *)checksum_buf,
                                  sizeof(pseudo) + sizeof(struct udphdr) + 5);
        
        sendto(raw_sock, out_buffer,
               sizeof(struct udphdr) + 5,
               0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        
        close(raw_sock);
    }
    
    if (udp_sock != -1) {
        close(udp_sock);
    }
    
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server_ip> [client_port]\n", argv[0]);
        fprintf(stderr, "Example: %s 127.0.0.1 50001\n", argv[0]);
        exit(1);
    }

    const char *server_ip = argv[1];
    
    if (argc >= 3) {
        client_port = atoi(argv[2]);
        if (client_port <= 0 || client_port > 65535) {
            fprintf(stderr, "Invalid port: %d\n", client_port);
            exit(1);
        }
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_aton(server_ip, &server_addr.sin_addr) == 0) {
        fprintf(stderr, "Invalid IP address: %s\n", server_ip);
        exit(1);
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_sock < 0) {
        perror("socket RAW");
        exit(1);
    }

    int reuse = 1;
    if (setsockopt(raw_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR (RAW)");
    }

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(client_port);
    
    if (bind(raw_sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        perror("bind RAW");
        close(raw_sock);
        exit(1);
    }

    socklen_t len = sizeof(client_addr);
    if (getsockname(raw_sock, (struct sockaddr *)&client_addr, &len) == 0) {
        int actual_port = ntohs(client_addr.sin_port);
        if (client_port == 0) {
            client_port = actual_port;
        }
        printf("Client bound to port %d\n", client_port);
    }

    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        perror("socket UDP");
        close(raw_sock);
        exit(1);
    }

    if (setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR (UDP)");
    }

    struct sockaddr_in udp_addr;
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(client_port);
    
    if (bind(udp_sock, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
        perror("bind UDP");
        close(raw_sock);
        close(udp_sock);
        exit(1);
    }

    printf("UDP socket bound to same port %d for receiving\n", client_port);
    printf("Ready to send messages. Type 'quit' to exit.\n\n");

    char sendline[BUFFER_SIZE], recvline[BUFFER_SIZE];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = udp_sock;
    fds[1].events = POLLIN;

    while (1) {
        int ret = poll(fds, 2, 1000);
        
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }

        if (fds[0].revents & POLLIN) {
            printf("> ");
            fflush(stdout);
            
            if (fgets(sendline, sizeof(sendline), stdin) == NULL) {
                printf("\nError reading input\n");
                break;
            }
            
            sendline[strcspn(sendline, "\n")] = 0;

            if (strcmp(sendline, "quit") == 0 || strcmp(sendline, "exit") == 0) {
                signal_handler(SIGINT);
                break;
            }

            if (strlen(sendline) == 0) {
                continue;
            }

            unsigned char out_buffer[BUFFER_SIZE];
            struct udphdr *udp_hdr = (struct udphdr *)out_buffer;
            udp_hdr->source = htons(client_port);
            udp_hdr->dest = htons(server_port);
            udp_hdr->len = htons(sizeof(struct udphdr) + strlen(sendline));
            udp_hdr->check = 0;

            memcpy(out_buffer + sizeof(struct udphdr), sendline, strlen(sendline));

            struct pseudo_header pseudo;
            pseudo.source_addr = client_addr.sin_addr.s_addr;
            pseudo.dest_addr = server_addr.sin_addr.s_addr;
            pseudo.placeholder = 0;
            pseudo.protocol = IPPROTO_UDP;
            pseudo.udp_length = udp_hdr->len;

            unsigned char checksum_buf[BUFFER_SIZE];
            memcpy(checksum_buf, &pseudo, sizeof(pseudo));
            memcpy(checksum_buf + sizeof(pseudo), out_buffer,
                   sizeof(struct udphdr) + strlen(sendline));
            udp_hdr->check = checksum((uint16_t *)checksum_buf,
                                      sizeof(pseudo) + sizeof(struct udphdr) + strlen(sendline));

            if (sendto(raw_sock, out_buffer,
                       sizeof(struct udphdr) + strlen(sendline),
                       0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("sendto");
                continue;
            }

            printf("Sent: '%s'\n", sendline);
        }

        if (fds[1].revents & POLLIN) {
            int n = recvfrom(udp_sock, recvline, sizeof(recvline) - 1, 0,
                             (struct sockaddr *)&from_addr, &from_len);
            if (n > 0) {
                recvline[n] = 0;
                printf("[SERVER] %s\n", recvline);
            }
        }
    }

    close(raw_sock);
    close(udp_sock);
    return 0;
}