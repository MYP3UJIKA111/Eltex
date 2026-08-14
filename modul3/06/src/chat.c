#include "udpchat.h"

static volatile sig_atomic_t g_stop = 0;

static void chat_signal_handler(int sig) {
    (void)sig;
    g_stop = 1;
}

int run_chat(void) {
    int sockfd;
    char buffer[MAX_MSG_SIZE];
    char hostname[HOSTNAME_MAX];
    pid_t my_pid = getpid();

    if (gethostname(hostname, sizeof(hostname)) < 0) {
        strncpy(hostname, "unknown", sizeof(hostname));
    }

    // Создание UDP-сокета
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    // Опции сокета
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        close(sockfd);
        return -1;
    }

    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("setsockopt SO_BROADCAST");
        close(sockfd);
        return -1;
    }

    // Привязка к порту
    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(CHAT_PORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    // Обработчики сигналов
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = chat_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Отправка JOIN
    snprintf(buffer, sizeof(buffer), "%s:%d:%s", MSG_JOIN, my_pid, hostname);
    send_broadcast(sockfd, buffer);

    printf("=== UDP Group Chat ===\n");
    printf("Joined as '%s' (pid=%d) on port %d\n", hostname, my_pid, CHAT_PORT);
    printf("Type messages and press Enter. '/exit' or Ctrl+C to quit.\n");
    printf("> ");
    fflush(stdout);

    // Главный цикл
    while (!g_stop) {
        fd_set rfds;
        struct timeval tv;

        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(sockfd, &rfds);

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(sockfd + 1, &rfds, NULL, NULL, &tv);

        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        if (ret == 0) continue;

        // Обработка входящих UDP-сообщений
        if (FD_ISSET(sockfd, &rfds)) {
            struct sockaddr_in sender;
            socklen_t sender_len = sizeof(sender);

            ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                 (struct sockaddr *)&sender, &sender_len);

            if (n > 0) {
                buffer[n] = '\0';

                char type[16];
                int sender_pid = 0;
                char sender_host[HOSTNAME_MAX];
                char text[MAX_MSG_SIZE];

                if (parse_message(buffer, type, &sender_pid,
                                  sender_host, sizeof(sender_host),
                                  text, sizeof(text)) == 0) {

                    if (sender_pid != my_pid) {
                        if (strcmp(type, MSG_JOIN) == 0) {
                            printf("\n>>> [%s] joined the chat\n", sender_host);
                        } else if (strcmp(type, MSG_LEAVE) == 0) {
                            printf("\n>>> [%s] left the chat\n", sender_host);
                        } else if (strcmp(type, MSG_TEXT) == 0 && text[0] != '\0') {
                            printf("\n[%s]: %s\n", sender_host, text);
                        }
                        printf("> ");
                        fflush(stdout);
                    }
                }
            }
        }

        // Обработка ввода пользователя
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                g_stop = 1;
                break;
            }

            buffer[strcspn(buffer, "\n")] = '\0';

            if (strcmp(buffer, "/exit") == 0 || strcmp(buffer, "exit") == 0) {
                g_stop = 1;
                break;
            }

            if (buffer[0] != '\0') {
                char msg[MAX_MSG_SIZE];
                snprintf(msg, sizeof(msg), "%s:%d:%s:%s",
                         MSG_TEXT, my_pid, hostname, buffer);

                if (send_broadcast(sockfd, msg) != 0) {
                    fprintf(stderr, "Failed to send message\n");
                }
            }

            printf("> ");
            fflush(stdout);
        }
    }

    // Отправка LEAVE
    snprintf(buffer, sizeof(buffer), "%s:%d:%s", MSG_LEAVE, my_pid, hostname);
    send_broadcast(sockfd, buffer);

    printf("\nLeft the chat. Goodbye!\n");

    close(sockfd);
    return 0;
}