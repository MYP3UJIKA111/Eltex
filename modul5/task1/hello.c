#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("Custom NGTU FAVT Galashina Educational License");
MODULE_AUTHOR("Galashina Anastasia Viktorovna");
MODULE_DESCRIPTION("A Custom Hello World module for Module5");

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello world! Module loaded successfully by Anastasia.\n");
    return 0;
}

static void __exit hello_cleanup(void)
{
    printk(KERN_INFO "Cleaning up module. Goodbye from kernel space!\n");
}

module_init(hello_init);
module_exit(hello_cleanup);