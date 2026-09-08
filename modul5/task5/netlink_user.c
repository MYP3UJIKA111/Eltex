#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_TEST 31
#define MAX_PAYLOAD 1024

int main() {
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    int sock_fd;
    struct msghdr msg;

    // Создаем Netlink сокет
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);
    if (sock_fd < 0) {
        perror("Ошибка создания сокета");
        return -1;
    }

    // Привязываем сокет к текущему процессу
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();

    if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
        perror("Ошибка bind");
        close(sock_fd);
        return -1;
    }

    // Настраиваем адрес получателя (Ядро = PID 0)
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = 0;

    // Выделяем память и формируем сообщение
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    strcpy(NLMSG_DATA(nlh), "Hello from userspace!");

    // Настраиваем структуру для отправки
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Отправка сообщения в ядро...\n");
    if (sendmsg(sock_fd, &msg, 0) < 0) {
        perror("Ошибка sendmsg");
        free(nlh);
        close(sock_fd);
        return -1;
    }

    printf("Ожидание ответа от ядра...\n");

    // Читаем ответ от ядра
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    if (recvmsg(sock_fd, &msg, 0) < 0) {
        perror("Ошибка recvmsg");
        free(nlh);
        close(sock_fd);
        return -1;
    }

    printf("Получено сообщение от ядра: %s\n", (char *)NLMSG_DATA(nlh));

    free(nlh);
    close(sock_fd);
    return 0;
}