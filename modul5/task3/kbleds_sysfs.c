#include <linux/module.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/console_struct.h>
#include <linux/vt_kern.h>
#include <linux/timer.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/spinlock.h>
#include <linux/kstrtox.h>

MODULE_DESCRIPTION("Keyboard LEDs blinker controlled via sysfs binary mask");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_VERSION("1.0");

#define BLINK_DELAY   (HZ / 2) // Мигание каждые 0.5 секунды
#define RESTORE_LEDS  0xFF     // Вернуть управление светодиодами системе 

static struct timer_list blink_timer;
static struct tty_driver *kb_driver;

static int led_mask = 0;          //* 0 = выкл, 1-7 = маска светодиодов
static int current_led_state = RESTORE_LEDS;
static DEFINE_SPINLOCK(mask_lock);

static struct kobject *kbleds_kobj;

static void blink_timer_func(struct timer_list *t)
{
    unsigned long flags;
    int target_mask;

    // Блокируем прерывания и захватываем спин-лок
    spin_lock_irqsave(&mask_lock, flags);
    target_mask = led_mask; 
    
    // Логика переключения состояния
    if (target_mask == 0) {
        current_led_state = RESTORE_LEDS;
    } else {
        // Если сейчас горит маска, гасим. Если погашено, зажигаем маску.
        if (current_led_state == target_mask) {
            current_led_state = RESTORE_LEDS;
        } else {
            current_led_state = target_mask;
        }
    }
    // Освобождаем блокировку
    spin_unlock_irqrestore(&mask_lock, flags);

    // Физическое управление клавиатурой через ioctl
    if (kb_driver && vc_cons[fg_console].d && vc_cons[fg_console].d->port.tty) {
        kb_driver->ops->ioctl(vc_cons[fg_console].d->port.tty, KDSETLED, current_led_state);
    }

    // Если мигание включено, планируем следующий запуск через 0.5 сек
    if (target_mask != 0) {
        blink_timer.expires = jiffies + BLINK_DELAY;
        add_timer(&blink_timer);
    }
}

static ssize_t led_mask_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    unsigned long flags;
    int val;
    
    spin_lock_irqsave(&mask_lock, flags);
    val = led_mask;
    spin_unlock_irqrestore(&mask_lock, flags);
    
    return sprintf(buf, "%d\n", val);
}

static ssize_t led_mask_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    unsigned long flags;
    int new_mask;
    
    // Безопасный парсинг числа из строки
    if (kstrtoint(buf, 10, &new_mask) != 0)
        return -EINVAL;

    // Валидация
    if (new_mask < 0 || new_mask > 7)
        return -EINVAL;

    // Критическая секция
    spin_lock_irqsave(&mask_lock, flags);
    led_mask = new_mask;
    
    if (led_mask > 0) {
        // Включаем мигание: сбрасываем состояние и запуск\перезапуск таймер
        current_led_state = RESTORE_LEDS;
        mod_timer(&blink_timer, jiffies + BLINK_DELAY);
    } else {
        // Выключаем мигание: таймер не перезапускается, но мы сразу гасим LEDs
        current_led_state = RESTORE_LEDS;
        if (kb_driver && vc_cons[fg_console].d && vc_cons[fg_console].d->port.tty) {
            kb_driver->ops->ioctl(vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);
        }
    }
    spin_unlock_irqrestore(&mask_lock, flags);

    return count; 
}

static struct kobj_attribute led_mask_attribute = 
    __ATTR(led_mask, 0660, led_mask_show, led_mask_store);

static int __init kbleds_sysfs_init(void)
{
    int error;
    pr_info("kbleds_sysfs: loading module...\n");

    // Проверка, что у нас есть доступ к активной консоли
    if (!vc_cons[fg_console].d || !vc_cons[fg_console].d->port.tty) {
        pr_err("kbleds_sysfs: Cannot get console TTY\n");
        return -ENODEV;
    }
    
    // Сохраняем указатель на драйвер клавиатуры для последующих ioctl
    kb_driver = vc_cons[fg_console].d->port.tty->driver;
    pr_info("kbleds_sysfs: tty driver acquired successfully\n");

    // Настраиваем таймер
    timer_setup(&blink_timer, blink_timer_func, 0);
    led_mask = 0;
    current_led_state = RESTORE_LEDS;

    // Создаем директорию /sys/kernel/kbleds_ctrl
    kbleds_kobj = kobject_create_and_add("kbleds_ctrl", kernel_kobj);
    if (!kbleds_kobj) return -ENOMEM;

    // Создаем внутри нее файл led_mask
    error = sysfs_create_file(kbleds_kobj, &led_mask_attribute.attr);
    if (error) {
        kobject_put(kbleds_kobj); // Откат при ошибке
        return error;
    }

    pr_info("kbleds_sysfs: loaded successfully. Control via /sys/kernel/kbleds_ctrl/led_mask\n");
    return 0;
}

static void __exit kbleds_sysfs_exit(void)
{
    unsigned long flags;

    pr_info("kbleds_sysfs: unloading module...\n");
    
    timer_delete_sync(&blink_timer);

    spin_lock_irqsave(&mask_lock, flags);
    if (kb_driver && vc_cons[fg_console].d && vc_cons[fg_console].d->port.tty) {
        kb_driver->ops->ioctl(vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);
    }
    spin_unlock_irqrestore(&mask_lock, flags);

    kobject_put(kbleds_kobj);
    pr_info("kbleds_sysfs: unloaded successfully\n");
}

module_init(kbleds_sysfs_init);
module_exit(kbleds_sysfs_exit);