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
#include "linux/of.h"       //设备树节点查询函数
#include "linux/ioport.h"   

#include "asm/io.h"         //IO操作
#include "linux/of_address.h"

#include "linux/slab.h"     //kmalloc() / vmalloc

/*
 * 日期：[2021.1.6] 
 * 作者：[xiongnuoye]
 * 项目：[基于device-tree驱动框架]
 * 功能：[1.实现了linux下基于device-tree的IO驱动]
 *      [2.采用pinctrl和gpio子系统方式描述节点信息]
 *      [2.完成用户态和内核态之间的数据交换]
 *      [3.熟练掌握printk的工作原理和使用方式]
 *      [4.The end!]
 */


#define DEVNUMS 1
#define DEVNAME "pinctrl-gpio"
#define DEVMAJOR   200
#define DEVMINOR   0
/* 虚实地址IO映射 */
void *__iomem CCGR1;
void *__iomem MUX_GPIO1_IO3;
void *__iomem PAD_GPIO1_IO3;
void *__iomem GPIO1_DR;
void *__iomem GPIO1_GDIR;
//这里采用的是 ”imx6ull“ OC 的IO 只是为了学习，没有实际意义


static int device_open(struct inode *obj, struct file *fp)
{
    printk(KERN_EMERG"device was opened!\r\n");
    return 0;
}

static int device_close(struct inode *obj, struct file *fp)
{
    printk(KERN_EMERG"device was closed!\r\n");
    return 0;
}

static ssize_t device_write(struct file *fp, const char __user *buf, size_t n, loff_t *lof)
{
    return 0;
}

static ssize_t device_read(struct file *fp, char __user *buf, size_t n, loff_t *lof)
{
    return 0;
}

/* 设备操作函数 */
static struct file_operations devicetreeops = {
    .owner   = THIS_MODULE,
    .open    = device_open,
    .release = device_close,
    .write   = device_write,
    .read    = device_read,
};

//设备
static struct{
    dev_t devid;
    int major;
    int minor;
    //struct cdev mycdev;
    struct class *myclass;
    struct device *mydevice;
    struct device_node *cur;
    struct property *per;
}devicetree;

static int __init device_init(void)
{
    int i;
    int len;
    int nums;
    u32 vals;
    unsigned int reg[10];
    //devicetree.cur = (struct device_node *)kmalloc(sizeof(struct device_node),GFP_KERNEL);
    /* IO Init && extern dev init */
    devicetree.cur = of_find_node_by_path("/virtul-io");
    if(devicetree.cur == NULL)//找想要操作的节点
    {
        printk(KERN_EMERG"Node can't be found!\r\n");
        return -1;
    }
    //找到节点后打印节点信息
    len = 13;
    nums = of_property_count_elems_of_size(devicetree.cur,"reg",sizeof(unsigned int));
    for(i=0;i<nums;i++)
    {
        of_property_read_u32_index(devicetree.cur,"reg",i,&reg[i]);
        printk(KERN_EMERG"reg[%d] = %#x\r\n",i,reg[i]);
    }
    devicetree.per = of_find_property(devicetree.cur,"compatible",&len);//获取13字节一字节8位，所以获取13个字符组成的串
    if(devicetree.per != NULL)
        printk(KERN_EMERG"node-interrupt-name: %s\r\n",(char*)(devicetree.per->value));
    len = 4;
    devicetree.per = of_find_property(devicetree.cur,"status",&len);//获取4字节一字节8位，所以获取4个字符组成的串
    if(devicetree.per != NULL)
        printk(KERN_EMERG"status: <%s>\r\n",(char*)(devicetree.per->value));
    
    // virtio_1 = of_iomap(devicetree.cur,0);
    // virtio_2 = of_iomap(devicetree.cur,1);
    MUX_GPIO1_IO3 = of_iomap(devicetree.cur,0);
    PAD_GPIO1_IO3 = of_iomap(devicetree.cur,1);
    GPIO1_DR      = of_iomap(devicetree.cur,2);
    GPIO1_GDIR    = of_iomap(devicetree.cur,3);
    CCGR1         = of_iomap(devicetree.cur,4);
    /* GPIO初始化 */
    vals = readl(CCGR1);//使能时钟
    vals |= (0x3<<26);
    writel(vals,CCGR1);
    //设置IO属性
    writel(0x5,MUX_GPIO1_IO3);
    writel(0x10B0,PAD_GPIO1_IO3);
    vals = readl(GPIO1_GDIR);
    vals |= (0x1<<3);
    writel(vals,GPIO1_GDIR);    //输出模式
    vals = readl(GPIO1_DR);
    vals |= (0x1<<3);
    writel(vals,GPIO1_DR);      //拉高IO

    /* mydev init */
    devicetree.major = DEVMAJOR;
    devicetree.minor = DEVMINOR;
    devicetree.devid = ((DEVMAJOR<<20)|DEVMINOR);
    /* 注册设备 */
    register_chrdev(devicetree.major,DEVNAME,&devicetreeops);
    /* 创建设备 */
    devicetree.myclass = class_create(THIS_MODULE,DEVNAME);//create class 
    if(IS_ERR(devicetree.myclass))
    {
        return PTR_ERR(devicetree.myclass);
    }
    devicetree.mydevice = device_create(devicetree.myclass,NULL,devicetree.devid,NULL,DEVNAME);
    if(IS_ERR(devicetree.mydevice))
    {
        return PTR_ERR(devicetree.mydevice);
    }
    return 0;
}

static void __exit device_exit(void)
{
    device_destroy(devicetree.myclass,devicetree.devid);
    class_destroy(devicetree.myclass);
    unregister_chrdev(devicetree.major,DEVNAME);
    /* IO release */
    // iounmap(virtio_1);
    // iounmap(virtio_2);
    iounmap(MUX_GPIO1_IO3);
    iounmap(PAD_GPIO1_IO3);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);
    iounmap(CCGR1);
}

//模块加载
module_init(device_init);
//模块注销
module_exit(device_exit);

//许可证 && 作者
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiongnuoye");