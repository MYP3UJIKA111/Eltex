#include "../include/tcpchat.h"

static volatile sig_atomic_t g_stop = 0;

static void server_signal_handler(int sig) {
    (void)sig;
    g_stop = 1;
}

int run_tcp_server(void) {
    int listen_fd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;
    client_t clients[MAX_CLIENTS];
    struct pollfd fds[MAX_CLIENTS + 1];
    char buffer[MAX_MSG_SIZE];
    int i, j, nready, sockfd;
    
    // Инициализация клиентов
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = 0;
        clients[i].sockfd = -1;
    }
    
    // Создание сокета
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return -1;
    }
    
    // Опции сокета
    int reuse = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    // Привязка
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TCP_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return -1;
    }
    
    // Ожидание подключений
    if (listen(listen_fd, 10) < 0) {
        perror("listen");
        close(listen_fd);
        return -1;
    }
    
    // Обработчик сигналов
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = server_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    printf("=== TCP Chat Server ===\n");
    printf("Server listening on port %d\n", TCP_PORT);
    printf("Max clients: %d\n", MAX_CLIENTS);
    printf("Press Ctrl+C to stop\n\n");
    
    // Инициализация poll
    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;
    
    while (!g_stop) {
        nready = poll(fds, MAX_CLIENTS + 1, 1000);
        
        if (nready < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }
        
        // Новое подключение
        if (fds[0].revents & POLLIN) {
            cli_len = sizeof(cli_addr);
            sockfd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_len);
            
            if (sockfd < 0) {
                perror("accept");
                continue;
            }
            
            // Поиск свободного слота
            for (i = 1; i <= MAX_CLIENTS; i++) {
                if (!clients[i-1].active) {
                    clients[i-1].sockfd = sockfd;
                    clients[i-1].active = 1;
                    strcpy(clients[i-1].hostname, inet_ntoa(cli_addr.sin_addr));
                    
                    fds[i].fd = sockfd;
                    fds[i].events = POLLIN;
                    
                    printf("[+] Client %s connected (fd=%d)\n", 
                           clients[i-1].hostname, sockfd);
                    
                    // Уведомляем остальных
                    snprintf(buffer, sizeof(buffer), 
                            "[SERVER] %s joined the chat\n", clients[i-1].hostname);
                    for (j = 1; j <= MAX_CLIENTS; j++) {
                        if (clients[j-1].active && j != i) {
                            send(clients[j-1].sockfd, buffer, strlen(buffer), 0);
                        }
                    }
                    
                    break;
                }
            }
            
            if (i > MAX_CLIENTS) {
                printf("[-] Too many clients, rejecting connection\n");
                close(sockfd);
            }
        }
        
        // Обработка данных от клиентов
        for (i = 1; i <= MAX_CLIENTS; i++) {
            if (fds[i].revents & POLLIN) {
                sockfd = clients[i-1].sockfd;
                memset(buffer, 0, sizeof(buffer));
                
                int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
                
                if (n <= 0) {
                    // Клиент отключился
                    printf("[-] Client %s disconnected\n", clients[i-1].hostname);
                    
                    // Уведомляем остальных
                    snprintf(buffer, sizeof(buffer), 
                            "[SERVER] %s left the chat\n", clients[i-1].hostname);
                    for (j = 1; j <= MAX_CLIENTS; j++) {
                        if (clients[j-1].active && j != i) {
                            send(clients[j-1].sockfd, buffer, strlen(buffer), 0);
                        }
                    }
                    
                    close(sockfd);
                    clients[i-1].active = 0;
                    clients[i-1].sockfd = -1;
                    fds[i].fd = -1;
                    continue;
                }
                
                // Пересылаем сообщение всем
                printf("[%s]: %.*s", clients[i-1].hostname, n, buffer);
                
                for (j = 1; j <= MAX_CLIENTS; j++) {
                    if (clients[j-1].active && j != i) {
                        send(clients[j-1].sockfd, buffer, n, 0);
                    }
                }
            }
        }
    }
    
    // Закрытие всех сокетов
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            close(clients[i].sockfd);
        }
    }
    close(listen_fd);
    
    printf("\nServer stopped.\n");
    return 0;
}