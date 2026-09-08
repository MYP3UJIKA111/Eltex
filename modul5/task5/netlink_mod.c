#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/net_namespace.h>
#include <linux/string.h>

#define NETLINK_TEST 31       // Пользовательский номер протокола (20-31 зарезервированы для тестов)
#define MAX_PAYLOAD 1024      // Максимальный размер сообщения

struct sock *nl_sk = NULL;

// Функция обратного вызова, вызываемая при получении сообщения из userspace
static void nl_data_ready(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello from kernel space!"; // Ответное сообщение
    int res;
    u32 pid;

    // Получаем заголовок netlink сообщения из буфера
    nlh = (struct nlmsghdr *)skb->data;
    
    // Логируем полученное сообщение
    printk(KERN_INFO "Netlink: получено сообщение от userspace: %s\n", (char *)nlmsg_data(nlh));
    
    // Получаем PID процесса, отправившего сообщение
    pid = nlh->nlmsg_pid;

    msg_size = strlen(msg);
    
    // Выделяем новый буфер для ответного сообщения
    skb_out = nlmsg_new(msg_size, GFP_KERNEL);
    if (!skb_out) {
        printk(KERN_ERR "Netlink: не удалось выделить память для skb\n");
        return;
    }

    // Заполняем заголовок ответного сообщения
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    
    // Копируем данные в буфер ответа
    strscpy(nlmsg_data(nlh), msg, msg_size + 1);

    // Отправляем сообщение обратно в userspace
    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_ERR "Netlink: ошибка при отправке сообщения в userspace (код %d)\n", res);
}

// Конфигурация netlink сокета
static struct netlink_kernel_cfg cfg = {
    .input = nl_data_ready,
};

static int __init netlink_init(void) {
    printk(KERN_INFO "Netlink: инициализация модуля...\n");
    
    // Создаем netlink сокет в ядре
    nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Netlink: ошибка создания сокета.\n");
        return -10;
    }
    
    printk(KERN_INFO "Netlink: модуль успешно загружен (протокол %d)\n", NETLINK_TEST);
    return 0;
}

static void __exit netlink_exit(void) {
    printk(KERN_INFO "Netlink: выгрузка модуля...\n");
    netlink_kernel_release(nl_sk);
}

module_init(netlink_init);
module_exit(netlink_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("Modern Netlink communication example");
MODULE_VERSION("1.0");