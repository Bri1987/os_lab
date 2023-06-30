// 1. 包含头文件
#include <linux/kernel.h>
#include <linux/module.h>

//添加模块信息，不加也行
//标明遵循 GPL 协议，
MODULE_LICENSE("GPL");
//模块编写作者信息
MODULE_AUTHOR("bri1987");
//模块功能描述
MODULE_DESCRIPTION("this a driver for XXXX device");
// 2. 模块入口函数实现 返回值为int
int init_mymodule(void)
{    //在内核模块编程中 打印函数为 printk()
//  打印输出信息，这个信息只有在控制台 输入命令 dmesg | tail
    printk(KERN_ALERT "Hello,Kernel!\n");
    return 0;
}
// 3. 模块出口函数实现，返回值必须为void
void exit_mymodule(void)
{
    printk(KERN_NOTICE "exit mymodule\n");
}
// 4. 这里是对内核这个内核模块函数的入口声明。
module_init(init_mymodule);
// 5. 这里是对内核模块出口函数的声明
module_exit(exit_mymodule);