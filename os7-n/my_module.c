#include <linux/module.h>
#include <linux/fs.h>

static int major_num = 244;

static void my_module_exit(void)
{
    // 注销字符设备驱动程序
    unregister_chrdev(major_num, "mychardev2");

    printk(KERN_INFO "mychardev2 unloaded\n");
}

module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("My Character Device Driver");