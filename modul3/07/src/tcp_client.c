#include "../include/tcpchat.h"

static volatile sig_atomic_t g_stop = 0;

static void client_signal_handler(int sig) {
    (void)sig;
    g_stop = 1;
}

int run_tcp_client(const char *server_ip) {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[MAX_MSG_SIZE];
    fd_set readfds;
    int max_fd;
    
    // Создание сокета
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    
    // Подключение к серверу
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TCP_PORT);
    
    if (inet_aton(server_ip, &serv_addr.sin_addr) == 0) {
        printf("Invalid IP address: %s\n", server_ip);
        close(sockfd);
        return -1;
    }
    
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }
    
    // Обработчик сигналов
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = client_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    printf("=== TCP Chat Client ===\n");
    printf("Connected to server %s:%d\n", server_ip, TCP_PORT);
    printf("Commands:\n");
    printf("  /file <filename> - send file\n");
    printf("  /exit            - disconnect\n");
    printf("  Ctrl+C           - disconnect\n");
    printf("> ");
    fflush(stdout);
    
    while (!g_stop) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds);
        max_fd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;
        
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        int ret = select(max_fd + 1, &readfds, NULL, NULL, &tv);
        
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        
        // Сообщение от сервера
        if (FD_ISSET(sockfd, &readfds)) {
            memset(buffer, 0, sizeof(buffer));
            int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            
            if (n <= 0) {
                printf("\n[SERVER] Disconnected from server\n");
                break;
            }
            
            printf("\n%s", buffer);
            printf("> ");
            fflush(stdout);
        }
        
        // Ввод пользователя
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                g_stop = 1;
                break;
            }
            
            buffer[strcspn(buffer, "\n")] = '\0';
            
            if (strcmp(buffer, "/exit") == 0) {
                g_stop = 1;
                break;
            }
            
            // Отправка файла
            if (strncmp(buffer, "/file ", 6) == 0) {
                char *filename = buffer + 6;
                if (send_file_data(sockfd, filename) != 0) {
                    printf("[ERROR] Failed to send file\n");
                }
                printf("> ");
                fflush(stdout);
                continue;
            }
            
            // Отправка сообщения
            if (buffer[0] != '\0') {
                if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
                    perror("send");
                }
            }
            
            printf("> ");
            fflush(stdout);
        }
    }
    
    close(sockfd);
    printf("\nDisconnected\n");
    return 0;
}

// Отправка файла
int send_file_data(int sockfd, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        return -1;
    }
    
    // Получаем размер файла
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Отправляем заголовок: FILE:<filename>:<size>
    char header[512];
    snprintf(header, sizeof(header), "FILE:%s:%ld\n", filename, file_size);
    
    if (send(sockfd, header, strlen(header), 0) < 0) {
        perror("send header");
        fclose(fp);
        return -1;
    }
    
    // Отправляем данные файла
    char buffer[1024];
    int bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (send(sockfd, buffer, bytes, 0) < 0) {
            perror("send file data");
            fclose(fp);
            return -1;
        }
    }
    
    fclose(fp);
    printf("[FILE] Sent '%s' (%ld bytes)\n", filename, file_size);
    return 0;
}