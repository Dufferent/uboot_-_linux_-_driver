#include "linux/types.h"    //多种驱动结构体定义在这个头文件中
#include "linux/delay.h"    //内核延时
#include "linux/module.h"   //驱动模块初始化
#include "linux/init.h"     //__init __exit关键字
#include "linux/kernel.h"
#include "linux/ide.h"

/*
 * 日期：[2021.1.2] 
 * 作者：[xiongnuoye]
 * 项目：[基本字符设备框架]
 * 功能：[1.实现了linux下基本的字符设备驱动]
 *      [2.完成用户态和内核态之间的数据交换]
 *      [3.熟练掌握printk的工作原理和使用方式]
 *      [4.The end!]
 */

//设备号
dev_t chdev;
int major;
int minor;
#define DEVNUMS 1
#define DEVNAME "chdevbase"
const char kernel_data[] = "Those are kernel data!";

static int chdev_open(struct inode *obj, struct file *fp)
{
    printk(KERN_EMERG"chdev was opened!\r\n");
    return 0;
}

static int chdev_close(struct inode *obj, struct file *fp)
{
    printk(KERN_EMERG"chdev was closed!\r\n");
    return 0;
}

static ssize_t chdev_write(struct file *fp, const char __user *buf, size_t n, loff_t *lof)
{
    int ret = 0;
    char tmp[32] = {0};
    ret = copy_from_user(tmp,buf,n);
    if(ret < 0)
        printk(KERN_EMERG"copy from user failed!\r\n");
    else
        printk(KERN_EMERG"rec:[%s]\r\n",tmp);
    return 0;
}

static ssize_t chdev_read(struct file *fp, char __user *buf, size_t n, loff_t *lof)
{
    int ret = 0;
    ret = copy_to_user(buf,kernel_data,n);
    if(ret < 0)
        printk(KERN_EMERG"copy to user failed!\r\n");
    else
        printk(KERN_EMERG"copy to user successful!\r\n");
    return 0;
}

/* 设备初始化 */
static struct file_operations chdevops = {
    .owner = THIS_MODULE,
    .open  = chdev_open,
    .release = chdev_close,
    .write = chdev_write,
    .read  = chdev_read,
};

static int __init chdev_init(void)
{
    /* 初始化模块第一步 */
    //申请设备号 [动态]
    /*
    int ret = alloc_chrdev_region(&chdev,0,DEVNUMS,DEVNAME);
    if(ret < 0)
    {
        printk(KERN_EMERG"alloc devnumber failed!\r\n");
        return -1;
    }
    major = (chdev>>20);
    minor = (chdev&(~(0xfff00000)));
    printk(KERN_EMERG"dev major is [%d]\tminor is [%d]\r\n",major,minor);
    */
    //注册设备
    int ret;
    major = 200;
    minor = 0;
    chdev = ((major<<20)|minor);

    ret = register_chrdev(major,DEVNAME,&chdevops);
    if(ret < 0)
    {
        printk(KERN_EMERG"register dev failed!\r\n");
    }
    return 0;
}

static void __exit chdev_exit(void)
{
    //unregister_chrdev_region(chdev,DEVNUMS);
    unregister_chrdev(major,DEVNAME);
}

//模块加载
module_init(chdev_init);
//模块注销
module_exit(chdev_exit);

//许可证 && 作者
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiongnuoye");