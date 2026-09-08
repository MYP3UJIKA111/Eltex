#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/inet.h>
#include <linux/rwlock.h>
#include <linux/slab.h>

#define PROC_FILE_NAME "ip_blacklist"
#define PROC_FILE_MODE 0666

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("Netfilter outgoing IP blacklist module");
MODULE_VERSION("1.0");

struct ip_node {
    __be32 ip_addr;
    struct list_head list;
};

static LIST_HEAD(blacklist);
static DEFINE_RWLOCK(blacklist_lock);

static struct nf_hook_ops nfho;
static struct proc_dir_entry *proc_entry;

static unsigned int hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *ip_header;
    __be32 dst_ip;
    struct ip_node *node;
    int drop = 0;

    if (!skb)
        return NF_ACCEPT;


    ip_header = ip_hdr(skb);
    if (!ip_header)
        return NF_ACCEPT;

    dst_ip = ip_header->daddr;

    // Блокируем список только на чтение для быстрой проверки
    read_lock(&blacklist_lock);
    list_for_each_entry(node, &blacklist, list) {
        if (node->ip_addr == dst_ip) {
            drop = 1;
            break;
        }
    }
    read_unlock(&blacklist_lock);

    if (drop) {
        printk(KERN_INFO "ip_filter: BLOCKED outgoing packet to %pI4\n", &dst_ip);
        return NF_DROP;
    }

    return NF_ACCEPT;
}


static int blacklist_seq_show(struct seq_file *m, void *v)
{
    struct ip_node *node;

    read_lock(&blacklist_lock);
    list_for_each_entry(node, &blacklist, list) {
        seq_printf(m, "%pI4\n", &node->ip_addr);
    }
    read_unlock(&blacklist_lock);

    return 0;
}

static int blacklist_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, blacklist_seq_show, NULL);
}


static ssize_t blacklist_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char kbuf[64];
    char cmd[16];
    char ip_str[32];
    __be32 ip_addr;
    struct ip_node *node, *tmp;
    int ret;

    if (count >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;
    
    kbuf[count] = '\0';

    if (sscanf(kbuf, "%15s %31s", cmd, ip_str) != 2)
        return -EINVAL;

    
    if (!in4_pton(ip_str, -1, (u8 *)&ip_addr, -1, NULL))
        return -EINVAL;

    write_lock(&blacklist_lock);

    if (strcmp(cmd, "add") == 0) {
        
        list_for_each_entry(node, &blacklist, list) {
            if (node->ip_addr == ip_addr) {
                write_unlock(&blacklist_lock);
                return -EEXIST;
            }
        }
        node = kmalloc(sizeof(*node), GFP_KERNEL);
        if (!node) {
            write_unlock(&blacklist_lock);
            return -ENOMEM;
        }
        node->ip_addr = ip_addr;
        list_add_tail(&node->list, &blacklist);
        printk(KERN_INFO "ip_filter: Added %pI4 to blacklist\n", &ip_addr);
        
    } else if (strcmp(cmd, "del") == 0) {
        ret = -ENOENT;
        list_for_each_entry_safe(node, tmp, &blacklist, list) {
            if (node->ip_addr == ip_addr) {
                list_del(&node->list);
                kfree(node);
                ret = count;
                printk(KERN_INFO "ip_filter: Removed %pI4 from blacklist\n", &ip_addr);
                break;
            }
        }
        write_unlock(&blacklist_lock);
        return ret == count ? count : -ENOENT;
        
    } else {
        write_unlock(&blacklist_lock);
        return -EINVAL;
    }

    write_unlock(&blacklist_lock);
    return count;
}


static const struct proc_ops filter_proc_ops = {
    .proc_open    = blacklist_seq_open,
    .proc_read    = seq_read,
    .proc_write   = blacklist_write,
    .proc_release = single_release,
};

static int __init ip_filter_init(void)
{

    nfho.hook     = hook_func;
    nfho.hooknum  = NF_INET_LOCAL_OUT; 
    nfho.pf       = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;

    if (nf_register_net_hook(&init_net, &nfho) != 0) {
        pr_err("ip_filter: Failed to register netfilter hook\n");
        return -1;
    }

    proc_entry = proc_create(PROC_FILE_NAME, PROC_FILE_MODE, NULL, &filter_proc_ops);
    if (!proc_entry) {
        nf_unregister_net_hook(&init_net, &nfho);
        pr_err("ip_filter: Failed to create proc entry\n");
        return -1;
    }

    pr_info("ip_filter: Module loaded. Manage via /proc/%s\n", PROC_FILE_NAME);
    return 0;
}

static void __exit ip_filter_exit(void)
{
    struct ip_node *node, *tmp;

    proc_remove(proc_entry);
    nf_unregister_net_hook(&init_net, &nfho);
    

    write_lock(&blacklist_lock);
    list_for_each_entry_safe(node, tmp, &blacklist, list) {
        list_del(&node->list);
        kfree(node);
    }
    write_unlock(&blacklist_lock);
    
    pr_info("ip_filter: Module unloaded\n");
}

module_init(ip_filter_init);
module_exit(ip_filter_exit);