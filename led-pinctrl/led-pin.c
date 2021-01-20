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
/* pinctrl & gpio-system */


/*
 * 日期：[2021.1.20] 
 * 作者：[xiongnuoye]
 * 项目：[新字符设备框架]
 * 功能：[1.实现了linux下新字符设备驱动]
 *      [2.完成用户态和内核态之间的数据交换]
 *      [3.熟练掌握printk的工作原理和使用方式]
 *      [4.掌握设备树的基本使用]
 *      [5.The end!]
 */

//设备
static struct{
    dev_t devid;
    struct class *devcl;
    struct device *devdv;
    struct device_node *node;
    int gpios;
}led;


#define DEVNUMS 1
#define DEVNAME "ledpin"
#define DEVMAJOR   200
#define DEVMINOR   0

void light(void)
{
    gpio_set_value(led.gpios,0);
}
void shutdown(void)
{
    gpio_set_value(led.gpios,1);
}

static int led_open(struct inode *obj, struct file *fp)
{
    printk(KERN_EMERG"ked was opened!\r\n");
    return 0;
}

static int led_close(struct inode *obj, struct file *fp)
{
    printk(KERN_EMERG"led was closed!\r\n");
    return 0;
}

static ssize_t led_write(struct file *fp, const char __user *buf, size_t n, loff_t *lof)
{
    unsigned char kbuf;
    int ret;
    ret = copy_from_user(&kbuf,buf,1);
    if(ret < 0)
    {
        printk(KERN_EMERG"write failed!\r\n");
        return -1;
    }
    switch(kbuf)
    {
        case 0:shutdown();printk(KERN_EMERG"led shutdown!\r\n");break;
        case 1:light();printk(KERN_EMERG"led light!\r\n");break;
        default:break;
    }
    return 0;
}

static ssize_t led_read(struct file *fp, char __user *buf, size_t n, loff_t *lof)
{
    printk(KERN_EMERG"no data for read!\r\n");
    return 0;
}

/* 设备操作函数 */
static struct file_operations ledops = {
    .owner = THIS_MODULE,
    .read  = led_read,
    .write = led_write,
    .open  = led_open,
    .release = led_close,
};

static int __init led_init(void)
{
    /* io 初始化 */
    int ret;
    /* find node */
    led.node = of_find_node_by_path("/led");
    if(led.node == NULL)
    {
        printk(KERN_EMERG"can't find node!\r\n");
        return -1;
    }
    /* pick gpios */
    led.gpios = of_get_named_gpio(led.node,"led-gpios",0);
    if(led.gpios < 0)
    {
        printk(KERN_EMERG"can't pick gpio nums!\r\n");
        return -1;
    }
    
    ret = gpio_request(led.gpios,"led");
    if(ret < 0)
    {
        printk(KERN_EMERG"request gpio failed!\r\n");
        return -1;       
    }
    
    gpio_direction_output(led.gpios, 0);

    /* 设备初始化 */
    ret = register_chrdev(DEVMAJOR,DEVNAME,&ledops);
    if(ret < 0)
    {
        printk(KERN_EMERG"regist failed!\r\n");
        return -1;
    }
    /* 创建设备 */
    led.devid  = (DEVMAJOR<<20)|(DEVMINOR);
    led.devcl  = class_create(THIS_MODULE,DEVNAME);
    led.devdv = device_create(led.devcl, NULL, led.devid , NULL, DEVNAME);
    return 0;
}

static void __exit led_exit(void)
{
    /* IO del */
    gpio_free(led.gpios);
    device_destroy(led.devcl,led.devid);
    class_destroy(led.devcl);
    unregister_chrdev(DEVMAJOR,DEVNAME);
}

//模块加载
module_init(led_init);
//模块注销
module_exit(led_exit);

//许可证 && 作者
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiongnuoye");