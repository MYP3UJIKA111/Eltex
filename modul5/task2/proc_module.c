#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define PROC_ENTRY_NAME "proc_module"
#define PROC_FILE_MODE  0666
#define MSG_BUFFER_SIZE 256

static char msg[MSG_BUFFER_SIZE];
static size_t msg_len = 0;
static struct proc_dir_entry *proc_entry;

static ssize_t hello_proc_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
    if (*offp > 0) {
        return 0;
    }

    if (count > msg_len) {
        count = msg_len;
    }

    if (copy_to_user(buf, msg, count)) {
        return -EFAULT;
    }

    *offp += count;
    return count;
}

static ssize_t hello_proc_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{

    if (count > MSG_BUFFER_SIZE - 1) {
        count = MSG_BUFFER_SIZE - 1;
    }

    if (copy_from_user(msg, buf, count)) {
        return -EFAULT;
    }

    msg[count] = '\0';
    msg_len = count;

    return count;
}

static const struct proc_ops hello_proc_ops = {
    .proc_read  = hello_proc_read,
    .proc_write = hello_proc_write,
};

static int __init hello_proc_init(void)
{
    proc_entry = proc_create(PROC_ENTRY_NAME, PROC_FILE_MODE, NULL, &hello_proc_ops);
    if (!proc_entry) {
        pr_err("Ошибка: не удалось создать запись в /proc\n");
        return -ENOMEM;
    }

    snprintf(msg, MSG_BUFFER_SIZE, "Hello from kernel module!\n");
    msg_len = strlen(msg);

    pr_info("Модуль hello_proc успешно загружен\n");
    return 0;
}

static void __exit hello_proc_exit(void)
{
    proc_remove(proc_entry);
    pr_info("Модуль proc_module выгружен\n");
}

module_init(hello_proc_init);
module_exit(hello_proc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student Galashina");
MODULE_DESCRIPTION("Modern procfs kernel module example without magic numbers and with static variables");
MODULE_VERSION("1.0");