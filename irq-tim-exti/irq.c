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

/* IRQ About */
#include "linux/of_irq.h"
#include "linux/irq.h"
#include "linux/irqdesc.h"
#include "linux/interrupt.h"
/* TIM About */
#include "linux/timer.h"
#include "linux/time.h"


/*
 * 日期：[2021.1.21] 
 * 作者：[xiongnuoye]
 * 项目：[新字符设备框架]
 * 功能：[1.实现了linux下新字符设备驱动]
 *      [2.完成用户态和内核态之间的数据交换]
 *      [3.熟练掌握printk的工作原理和使用方式]
 *      [4.掌握设备树的基本使用]
 *      [5.The end!]
 */

#define DEVNUMS 2
#define DEVNAME "irq-ctrl"
#define DEVMAJOR   200
#define DEVMINOR   0

//设备
static struct{
    dev_t devid;
    struct class *devcl;
    struct device *devdv;
    struct device_node *(node[DEVNUMS]);
    int gpios[DEVNUMS];
    int irqnums;
}irq_ctrl;

static irqreturn_t key_irqhandler(int irq,void *arg)
{
    static int tol = 1;
    printk(KERN_EMERG"key press!\r\n");
    if(tol)
    {
        gpio_set_value(irq_ctrl.gpios[0],1);
        tol = 0;
    }
    else
    {
        gpio_set_value(irq_ctrl.gpios[0],0);
        tol = 1;
    }
    return IRQ_RETVAL(IRQ_HANDLED);
}

static ssize_t dev_write(struct file *fp, const char __user *buf, size_t sz, loff_t *offt)
{
    char tmp;
    int ret;
    ret = copy_from_user(&tmp,buf,1);
    if(ret < 0)
    {
        printk(KERN_EMERG"rec user data failed!\r\n");
        return -1;
    }
    if(tmp == 1)
        enable_irq(irq_ctrl.irqnums);
    else
        disable_irq(irq_ctrl.irqnums);
    return 0;
}

static int dev_open (struct inode *np, struct file *fp)
{
    printk(KERN_EMERG"dev opened!\r\n");
    return 0;
}

static int dev_release (struct inode *np, struct file *fp)
{
    printk(KERN_EMERG"dev closed!\r\n");
    return 0;
}

/* 设备操作函数 */
static struct file_operations irqops = {
    .owner = THIS_MODULE,
    .open  = dev_open,
    .release = dev_release,
    .write   = dev_write,
};

static int __init irqdev_init(void)
{
    int ret = 0;
    /* led */
    irq_ctrl.node[0] = of_find_node_by_path("/led");
    if(irq_ctrl.node == NULL)
    {
        printk(KERN_EMERG"can't find node!\r\n");
        return -1;
    }
    irq_ctrl.gpios[0] = of_get_named_gpio(irq_ctrl.node[0],"led-gpios",0);
    if(irq_ctrl.gpios[0] < 0)
    {
        printk(KERN_EMERG"can't get led's gpio!\r\n");
        return -1;        
    }
    gpio_request(irq_ctrl.gpios[0],"led");
    gpio_direction_output(irq_ctrl.gpios[0],0);
    /* key */
    irq_ctrl.node[1] = of_find_node_by_path("/key");
    if(irq_ctrl.node == NULL)
    {
        printk(KERN_EMERG"can't find node!\r\n");
        return -1;
    }
    irq_ctrl.gpios[1] = of_get_named_gpio(irq_ctrl.node[1],"key-gpios",0);
    if(irq_ctrl.gpios[1] < 0)
    {
        printk(KERN_EMERG"can't get key's gpio!\r\n");
        return -1;        
    }
    gpio_request(irq_ctrl.gpios[1],"key");
    gpio_direction_input(irq_ctrl.gpios[1]);
    /* 申请中断 */
    irq_ctrl.irqnums = of_irq_get(irq_ctrl.node[1],0);
    ret = request_irq(irq_ctrl.irqnums, key_irqhandler, IRQ_TYPE_EDGE_BOTH, "exti", NULL);
    if(ret < 0)
    {
        printk(KERN_EMERG"exti irq request failed!\r\n");
        return -1;         
    }
    /* 注册设备 */
    irq_ctrl.devid = ((DEVMAJOR<<20)|DEVMINOR);
    register_chrdev(DEVMAJOR,DEVNAME,&irqops);
    irq_ctrl.devcl = class_create(THIS_MODULE,DEVNAME);
    irq_ctrl.devdv = device_create(irq_ctrl.devcl, NULL,irq_ctrl.devid, NULL, DEVNAME);

    return 0;
}

static void __exit irqdev_exit(void)
{
    disable_irq(irq_ctrl.irqnums);
    free_irq(irq_ctrl.irqnums,&irq_ctrl.devid);
    gpio_free(irq_ctrl.gpios[0]);
    gpio_free(irq_ctrl.gpios[1]);
    device_destroy(irq_ctrl.devcl,irq_ctrl.devid);
    class_destroy(irq_ctrl.devcl);
    unregister_chrdev(DEVMAJOR,DEVNAME);
}

//模块加载
module_init(irqdev_init);
//模块注销
module_exit(irqdev_exit);

//许可证 && 作者
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiongnuoye");