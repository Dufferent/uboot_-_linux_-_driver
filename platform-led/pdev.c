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
/* IO About */
#include "linux/gpio.h"
#include "asm/io.h"
#include "asm/gpio.h"
#include "asm/mman.h"
/* OF About */
#include "linux/of.h"
#include "linux/of_gpio.h"
/* PLATFORM About */
#include "linux/platform_device.h"

#define DEVNAME  "pdev"
#define DEVMAJOR 200
#define DEVMINOR 0
#define DEVNUMS  1

struct {
    dev_t devid;
    struct device *ddev;
    struct class  *dcls;
    struct device_node *np;
    int gpios;
}mydev;

int dev_open (struct inode *np, struct file *fp)
{
    printk(KERN_EMERG"dev was opened!\r\n");
    return 0;
}

int dev_release (struct inode *np, struct file *fp)
{
    printk(KERN_EMERG"dev was closed!\r\n");
    return 0;
}

ssize_t dev_write (struct file *fp, const char __user *buf, size_t sz, loff_t *ofset)
{
    char tmps;
    int ret = 0;
    ret = copy_from_user(&tmps, buf, 1);
    if(ret < 0)
    {
        printk(KERN_EMERG"User write failed!\r\n");
        return -1; 
    }
    if(tmps == 0)
        gpio_set_value(mydev.gpios,0);  //亮
    else
        gpio_set_value(mydev.gpios,1);  //灭
    return 0;
}

struct file_operations myfops = {
    .owner = THIS_MODULE,
    .open  = dev_open,
    .release = dev_release,
    .write = dev_write,
};

int dev_probe (struct platform_device *pdev)
{
    int ret = 0;
    /* 设备初始化 */
    mydev.np = of_find_node_by_path("/led");
    if(mydev.np == NULL)
    {
        printk(KERN_EMERG"find node failed!\r\n");
        return -1;
    }
    mydev.gpios = of_get_named_gpio(mydev.np,"led-gpios",0);
    if(mydev.gpios < 0)
    {
        printk(KERN_EMERG"find gpios failed!\r\n");
        return -1;        
    }
    gpio_request(mydev.gpios,"led");
    gpio_direction_output(mydev.gpios,0);
    /* 注册设备 */
    mydev.devid = (DEVMAJOR<<20)|(DEVMINOR);
    ret = register_chrdev(DEVMAJOR,DEVNAME,&myfops);
    if(ret < 0)
    {
        printk(KERN_EMERG"pdev regist failed!\r\n");
        return -1;
    }
    mydev.dcls = class_create(THIS_MODULE,DEVNAME);
    mydev.ddev = device_create(mydev.dcls, NULL, mydev.devid, NULL, DEVNAME);
    return 0;
}

int dev_remove (struct platform_device *pdev)
{
    gpio_free(mydev.gpios);
    device_destroy(mydev.dcls,mydev.devid);
    class_destroy(mydev.dcls);
    unregister_chrdev(DEVMAJOR,DEVNAME);
    return 0;
}

const struct of_device_id pdev_match_table[] = {
    {.compatible = "fsl,led"},
    {},
};

struct platform_driver mypldev = {
    .probe  = dev_probe,
    .remove = dev_remove,
    .driver = {
        .name  = DEVNAME,
        .owner = THIS_MODULE,
        .of_match_table = pdev_match_table,
    },
    /*
    .id_table = {

    },
    */
};

static int __init platform_driver_init(void)
{
    return platform_driver_register(&mypldev);
}

static void __exit platform_driver_exit(void)
{
    platform_driver_unregister(&mypldev);
}

module_init(platform_driver_init);
module_exit(platform_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiongnuoye");