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

/*
 * 日期：[2021.1.20] 
 * 作者：[xiongnuoye]
 * 项目：[新字符设备框架]
 * 功能：[1.实现了linux下新字符设备驱动]
 *      [2.完成用户态和内核态之间的数据交换]
 *      [3.熟练掌握printk的工作原理和使用方式]
 *      [4.The end!]
 */


#define DEVNUMS 1
#define DEVNAME "leddev"
#define DEVMAJOR   200
#define DEVMINOR   0

#define CCM_CCGR1 (0x020C406C)
#define MUX_GPIO1 (0x020E0068)
#define PAD_GPIO1 (0x020E02F4)
#define GPIO1_DR  (0X0209C000)
#define GPIO1_DIR (0X0209C004)

static void *__iomem VIR_CCM_CCGR1;
static void *__iomem VIR_MUX_GPIO1;
static void *__iomem VIR_PAD_GPIO1;
static void *__iomem VIR_GPIO1_DR;
static void *__iomem VIR_GPIO1_DIR;

void light(void)
{
    unsigned int val = readl(VIR_GPIO1_DR);
    val &= (~(0x1<<3));
    writel(val,VIR_GPIO1_DR);
}
void shutdown(void)
{
    unsigned int val = readl(VIR_GPIO1_DR);
    val |= (0x1<<3);
    writel(val,VIR_GPIO1_DR);
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

//设备
static struct{
    dev_t devid;
    struct class *devcl;
    struct device *devdv;
}led;

static int __init led_init(void)
{
    /* io 初始化 */
    unsigned int val = 0;
    int ret;
    VIR_CCM_CCGR1 = ioremap(CCM_CCGR1,4);
    VIR_MUX_GPIO1 = ioremap(MUX_GPIO1,4);
    VIR_PAD_GPIO1 = ioremap(PAD_GPIO1,4);
    VIR_GPIO1_DR  = ioremap(GPIO1_DR ,4);
    VIR_GPIO1_DIR = ioremap(GPIO1_DIR,4);
    val = readl(VIR_CCM_CCGR1);
    val |= (0x3<<26);
    writel(val,VIR_CCM_CCGR1);
    writel(0x5,VIR_MUX_GPIO1);
    writel(0x10B0,VIR_PAD_GPIO1);
    val = readl(VIR_GPIO1_DIR);
    val |= (0x1<<3);
    writel(val,VIR_GPIO1_DIR);
    val = readl(VIR_GPIO1_DR);
    val &= (~(0x1<<3));
    writel(val,VIR_GPIO1_DR);//亮灯
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