#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>

#include "monitor_ioctl.h"

#define DEVICE_NAME "container_monitor"

static int major;

/* simple structure to track one process */
struct tracked_proc {
    pid_t pid;
    unsigned long soft;
    unsigned long hard;
    struct list_head list;
};

static LIST_HEAD(proc_list);
static DEFINE_MUTEX(proc_lock);

/* ioctl handler */
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct monitor_request req;
    struct tracked_proc *p;

    if (copy_from_user(&req, (void __user *)arg, sizeof(req)))
        return -EFAULT;

    mutex_lock(&proc_lock);

    if (cmd == MONITOR_REGISTER) {
        p = kmalloc(sizeof(*p), GFP_KERNEL);
        if (!p) {
            mutex_unlock(&proc_lock);
            return -ENOMEM;
        }

        p->pid = req.pid;
        p->soft = req.soft_limit_bytes;
        p->hard = req.hard_limit_bytes;

        list_add(&p->list, &proc_list);

        printk(KERN_INFO "Monitor: Registered PID %d\n", p->pid);
    }

    if (cmd == MONITOR_UNREGISTER) {
        list_for_each_entry(p, &proc_list, list) {
            if (p->pid == req.pid) {
                list_del(&p->list);
                kfree(p);
                printk(KERN_INFO "Monitor: Unregistered PID %d\n", req.pid);
                break;
            }
        }
    }

    mutex_unlock(&proc_lock);
    return 0;
}

/* basic file ops */
static struct file_operations fops = {
    .unlocked_ioctl = device_ioctl,
};



static void check_memory(void)
{
    struct tracked_proc *p;
    struct task_struct *task;

    mutex_lock(&proc_lock);

    list_for_each_entry(p, &proc_list, list) {
        task = pid_task(find_vpid(p->pid), PIDTYPE_PID);
        if (!task)
            continue;

        if (task->mm) {
            unsigned long rss = get_mm_rss(task->mm) << PAGE_SHIFT;

            if (rss > p->hard) {
                printk(KERN_INFO "Monitor: Killing PID %d (RSS=%lu)\n",
                       p->pid, rss);
                send_sig(SIGKILL, task, 0);
            }
        }
    }

    mutex_unlock(&proc_lock);
}

static struct timer_list monitor_timer;

static void timer_callback(struct timer_list *t)
{
    check_memory();
    mod_timer(&monitor_timer, jiffies + msecs_to_jiffies(1000));
}


/* module init */
static int __init monitor_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ERR "Failed to register device\n");
        return major;
    }

    timer_setup(&monitor_timer, timer_callback, 0);
    mod_timer(&monitor_timer, jiffies + msecs_to_jiffies(1000));

    printk(KERN_INFO "Monitor module loaded\n");
    return 0;
}

/* module exit */
static void __exit monitor_exit(void)
{
    unregister_chrdev(major, DEVICE_NAME);

    del_timer(&monitor_timer);

    printk(KERN_INFO "Monitor module unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);

MODULE_LICENSE("GPL");
