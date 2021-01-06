#include "linux/types.h"    //多种驱动结构体定义在这个头文件中
#include "linux/delay.h"    //内核延时
#include "linux/module.h"   //驱动模块初始化
#include "linux/init.h"     //__init __exit关键字
#include "linux/kernel.h"
#include "linux/ide.h"
#include "linux/cdev.h"     //cdev结构体头文件
#include "linux/device.h"   //device结构体头文件,class结构体头文件
#include "linux/errno.h"    //错误处理 
#include "asm/uaccess.h"

/*
 * 日期：[2021.1.3] 
 * 作者：[xiongnuoye]
 * 项目：[新字符设备框架]
 * 功能：[1.实现了linux下新字符设备驱动]
 *      [2.完成用户态和内核态之间的数据交换]
 *      [3.熟练掌握printk的工作原理和使用方式]
 *      [4.The end!]
 */


#define DEVNUMS 1
#define DEVNAME "newchdev"
#define DEVMAJOR   200
#define DEVMINOR   0

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

/* 设备操作函数 */
static struct file_operations chdevops = {
    .owner   = THIS_MODULE,
    .open    = chdev_open,
    .release = chdev_close,
    .write   = chdev_write,
    .read    = chdev_read,
};

//设备
static struct{
    dev_t devid;
    int major;
    int minor;
    //struct cdev mycdev;
    struct class *myclass;
    struct device *mydevice;
}newchdev;

static int __init chdev_init(void)
{
    /* mydev init */
    newchdev.major = DEVMAJOR;
    newchdev.minor = DEVMINOR;
    newchdev.devid = ((DEVMAJOR<<20)|DEVMINOR);
    /* 注册设备 */
    register_chrdev(newchdev.major,DEVNAME,&chdevops);
    /* 创建设备 */
    newchdev.myclass = class_create(THIS_MODULE,DEVNAME);//create class 
    if(IS_ERR(newchdev.myclass))
    {
        return PTR_ERR(newchdev.myclass);
    }
    newchdev.mydevice = device_create(newchdev.myclass,NULL,newchdev.devid,NULL,DEVNAME);
    if(IS_ERR(newchdev.mydevice))
    {
        return PTR_ERR(newchdev.mydevice);
    }
    return 0;
}

static void __exit chdev_exit(void)
{
    device_destroy(newchdev.myclass,newchdev.devid);
    class_destroy(newchdev.myclass);
    unregister_chrdev(newchdev.major,DEVNAME);
}

//模块加载
module_init(chdev_init);
//模块注销
module_exit(chdev_exit);

//许可证 && 作者
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiongnuoye");