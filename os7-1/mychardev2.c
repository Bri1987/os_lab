#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/sched.h>
#include <linux/init_task.h>

#define DEVICE_NAME "mychardev2"
#define BUF_LEN 1024
#define MAJOR_NUM 244
#define MINOR_NUM 0
static int Major;        //函数将返回分配的设备号
static char msg[BUF_LEN];
static char *msg_Ptr;    //msg_Ptr指向设备文件中的数据缓冲区
static dev_t devno;

//内核在内部使用inode结构来表示文件
//每个文件每打开一次就对应一个file结构，但是inode 结构只有一个
//在设备文件被打开时被调用。
static int device_open(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "mychardev2: device opened\n");
    //将设备文件的指针存储在文件私有数据结构中
    msg_Ptr = msg;
    try_module_get(THIS_MODULE);
    return 0;
}

//在设备文件被关闭时被调用
static int device_release(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "mychardev2: device closed\n");
    //减少模块的引用计数
    module_put(THIS_MODULE);
    return 0;
}

//从设备文件中读取数据，并使用put_user()函数将数据复制到用户空间缓冲区中
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    int bytes_read = 0;
    //当还有数据可以读取且缓冲区长度不为0时，继续读取
    while (length && *msg_Ptr) {
        //将一个字节的数据复制到用户空间缓冲区中
        put_user(*(msg_Ptr++), buffer++);
        length--;
        bytes_read++;
    }
    printk(KERN_ALERT "mychardev2: device read\n");
    return bytes_read;
}

//从用户空间缓冲区中读取数据，并使用get_user()函数将数据复制到设备文件中
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
    int i;
    for (i = 0; i < len && i < BUF_LEN; i++)
        get_user(msg[i], buff + i);
    msg_Ptr = msg;
    printk(KERN_ALERT "mychardev2: device write\n");
    return i;
}

//通过ioctl命令，用户空间程序可以向设备驱动程序发送指令，让设备驱动程序执行相应的操作
//可以使用ioctl命令来设置设备的属性、查询设备的状态、控制设备的操作等
//cmd：表示ioctl命令的编号。
//arg：表示ioctl命令的参数。
static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct task_struct *p;
    switch (cmd) {
        case 0:
            //获取init进程的进程控制块，并使用for_each_process()函数遍历进程控制块链表。在每次循环中，使用printk()函数输出进程的PID，状态，优先级和父进程ID等信息。
            printk(KERN_ALERT "mychardev2: device ioctl cmd=0\n");
            p = &init_task;
            for_each_process(p)
            {
                printk(KERN_ALERT
                "mychardev2: PID=%d, state=%ld, priority=%ld, parent=%d\n", p->pid, p->state, p->prio, p->parent->pid);
            }
        break;

        default:
            printk(KERN_ALERT "mychardev2: device ioctl unknown cmd=%d\n", cmd);
            return -EINVAL;
    }
    return 0;
}

//file_operations结构把驱动程序的操作和对设备文件操作联系在一起
static struct file_operations fops = {
        .read = device_read,
        .write = device_write,
        .unlocked_ioctl = device_ioctl,
        .open = device_open,
        .release = device_release,
};

//注册字符设备，并将字符设备的操作函数指针&fops传递给注册函数。register_chrdev()函数的第一个参数（0）表示告诉内核让内核自己分配设备号，第二个参数是一个字符串，表示字符设备的名称，第三个参数是字符设备的操作函数指针。
static int __init mychardev2_init(void)
{
    //通过MAJOR_NUM和MINOR_NUM宏定义得到设备号
    devno=MKDEV(MAJOR_NUM,MINOR_NUM);

    //如果第一个参数（设备号）为0，则表示让内核自动分配设备号
    //函数将返回分配的设备号
    Major = register_chrdev(244, DEVICE_NAME, &fops);

    if (Major < 0) {
        printk(KERN_ALERT "mychardev2: failed to register a major number\n");
        return Major;
    }
    printk(KERN_ALERT "mychardev2: registered successfully with major number %d\n", Major);
    return 0;
}

static void __exit mychardev2_exit(void)
{
    unregister_chrdev(244, DEVICE_NAME);
    unregister_chrdev_region(devno, 1);      //释放设备号
    printk(KERN_ALERT "mychardev2: unregistered successfully\n");
}

module_init(mychardev2_init);
module_exit(mychardev2_exit);