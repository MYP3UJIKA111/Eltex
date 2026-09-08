#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/string.h>

#define DEVICE_NAME "chardev"
#define CLASS_NAME "chardev_class"
#define BUF_LEN 256

static int major_number;
static dev_t dev_num;
static struct class *dev_class = NULL;
static struct device *dev_device = NULL;
static struct cdev chardev_cdev;

static char device_buffer[BUF_LEN];
static size_t buffer_len = 0;
static DEFINE_MUTEX(chardev_mutex);


static int chardev_open(struct inode *inode, struct file *file) {
    pr_info("chardev: Device opened\n");
    return 0;
}

static int chardev_release(struct inode *inode, struct file *file) {
    pr_info("chardev: Device closed\n");
    return 0;
}

static ssize_t chardev_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    if (*offset >= buffer_len)
        return 0;

    if (len > buffer_len - *offset)
        len = buffer_len - *offset;

    if (copy_to_user(buf, device_buffer + *offset, len))
        return -EFAULT;

    *offset += len;
    return len;
}

static ssize_t chardev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    if (len > BUF_LEN - 1)
        len = BUF_LEN - 1;

    if (copy_from_user(device_buffer, buf, len))
        return -EFAULT;

    device_buffer[len] = '\0';
    buffer_len = len;

    pr_info("chardev: Received %zu bytes from userspace\n", len);
    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = chardev_open,
    .release = chardev_release,
    .read = chardev_read,
    .write = chardev_write,
};


static int __init chardev_init(void) {
    pr_info("chardev: Initializing module...\n");

    // Динамическое выделение major номера
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        pr_alert("chardev: Failed to allocate major number\n");
        return -1;
    }
    major_number = MAJOR(dev_num);
    pr_info("chardev: Registered with major number %d\n", major_number);

    // Инициализация cdev
    cdev_init(&chardev_cdev, &fops);
    chardev_cdev.owner = THIS_MODULE;
    if (cdev_add(&chardev_cdev, dev_num, 1) < 0) {
        unregister_chrdev_region(dev_num, 1);
        pr_alert("chardev: Failed to add cdev\n");
        return -1;
    }

    // Создание класса 
    dev_class = class_create(CLASS_NAME);
    if (IS_ERR(dev_class)) {
        cdev_del(&chardev_cdev);
        unregister_chrdev_region(dev_num, 1);
        pr_alert("chardev: Failed to create class\n");
        return PTR_ERR(dev_class);
    }

    // Создание устройства
    dev_device = device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(dev_device)) {
        class_destroy(dev_class);
        cdev_del(&chardev_cdev);
        unregister_chrdev_region(dev_num, 1);
        pr_alert("chardev: Failed to create device\n");
        return PTR_ERR(dev_device);
    }

    pr_info("chardev: Device created at /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit chardev_exit(void) {
    pr_info("chardev: Unloading module...\n");
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&chardev_cdev);
    unregister_chrdev_region(dev_num, 1);
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("Modern character device driver for kernel 7.x");
MODULE_VERSION("1.0");