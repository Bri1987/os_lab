#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/sched.h>

#define BUF_LEN 1000000
#define MYCHARDEV3_IOCTL_CMD 0
#define MAJOR 242

static char *buf = NULL;
static struct mutex lock;

static int mychardev3_open(struct inode *inode, struct file *filp)
{
    int err = 0;

    mutex_lock(&lock);
    if (!buf) {
        buf = kmalloc(BUF_LEN, GFP_KERNEL);
        if (!buf) {
            printk(KERN_ERR "mychardev3: failed to allocate shared memory\n");
            err = -ENOMEM;
            goto fail;
        }
        memset(buf, 0, BUF_LEN);
    }
    filp->private_data = buf;
    mutex_unlock(&lock);

    return 0;

    fail:
    mutex_unlock(&lock);
    return err;
}

static long mychardev3_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    if (cmd != MYCHARDEV3_IOCTL_CMD) {
        ret = -EINVAL;
        printk(KERN_ERR "mychardev3: invalid IOCTL command\n");
        goto out;
    }

    struct task_struct *p;
    char *msg_ptr = buf;

    mutex_lock(&lock);

    for_each_process(p) {
        msg_ptr += sprintf(msg_ptr, "PID=%d, state=%ld, priority=%ld, parent=%d\n", p->pid, (long)p->state, (long)p->prio, p->parent ? p->parent->pid : -1);
        printk(KERN_ALERT
        "mychardev3: PID=%d, state=%ld, priority=%ld, parent=%d\n", p->pid, (long)p->state, (long)p->prio, p->parent->pid);
        if (msg_ptr - buf >= BUF_LEN) break;
    }

    mutex_unlock(&lock);

    out:
    return ret;
}

static ssize_t mychardev3_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
char *data = filp->private_data;
size_t len = strlen(data);
ssize_t ret = 0;

if (count == 0 || *pos >= len) goto out; // 处理读取长度为0和已经读完的情况
if (*pos + count > len) count = len - *pos; // 缩小读取长度，避免越界访问
if (copy_to_user(buf, data + *pos, count)) {
ret = -EFAULT;
goto out;
}
*pos += count;
ret = count;

out:
return ret;
}

static ssize_t mychardev3_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos)
{
char *data = filp->private_data;
ssize_t ret = 0;

if (*pos + count > BUF_LEN) count = BUF_LEN - *pos; // 缩小写入长度，避免越界访问
if (copy_from_user(data + *pos, buf, count)) {
ret = -EFAULT;
goto out;
}
*pos += count;
ret = count;

out:
return ret;
}

static int mychardev3_release(struct inode *inode, struct file *filp)
{
    kfree(filp->private_data);
    filp->private_data = NULL;

    return 0;
}

static struct file_operations mychardev3_fops = {
        .owner   = THIS_MODULE,
        .open    = mychardev3_open,
        .unlocked_ioctl = mychardev3_ioctl,
        .read    = mychardev3_read,
        .write   = mychardev3_write,
        .release = mychardev3_release,
};

static int __init mychardev3_init(void)
{
    int ret = 0;

    mutex_init(&lock);

    ret = register_chrdev(MAJOR, "mychardev3", &mychardev3_fops);
    if (ret < 0) {
        printk(KERN_ERR "mychardev3: failed to register character device\n");
        goto fail;
    }

    printk(KERN_INFO "mychardev3: character device registered\n");

    return 0;
    fail:
    mutex_destroy(&lock);

    return ret;
}

static void __exit mychardev3_exit(void)
{
    unregister_chrdev(MAJOR, "mychardev3");
    //mutex_destroy(&lock);
    //if (buf) kfree(buf);

    printk(KERN_INFO "mychardev3: module unloaded\n");
}

module_init(mychardev3_init);
module_exit(mychardev3_exit);

MODULE_AUTHOR("Little B");
MODULE_DESCRIPTION("A simple character device driver for reading process information");
MODULE_LICENSE("GPL");