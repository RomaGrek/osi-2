#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
 
#include <linux/path.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/netdevice.h>
#include <linux/blk_types.h>
#include <linux/namei.h>
#include <linux/ioctl.h>
#include <linux/pid.h>
#include <linux/sched.h>
 
#include <linux/ptrace.h>
#include <asm/syscall.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <stdbool.h>
#define WR_AVALUE _IOW('a','a',struct message*)
#define WR_SVALUE _IOW('a','c',struct message*)
#define RD_VALUE _IOR('a','b',struct message*)
#define BUF_LENGHT 1024
 
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("ioctl_driver");
MODULE_VERSION("1.0");
 
dev_t dev = 0;
int pid = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
 
struct my_block_device
{
    int bd_openers;
    int bd_holders;
    bool bd_read_only;
    int bd_fsfreeze_count;

};

enum my_memblock_flags {
	MEMBLOCK_NONE		= 0x0,	/* No special request */
	MEMBLOCK_HOTPLUG	= 0x1,	/* hotpluggable region */
	MEMBLOCK_MIRROR		= 0x2,	/* mirrored region */
	MEMBLOCK_NOMAP		= 0x4,	/* don't add to kernel direct mapping */
	MEMBLOCK_DRIVER_MANAGED = 0x8,	/* always detected via a driver */
};


struct my_memblock_region {
        int test;
};

struct my_memblock_type
{
        unsigned long cnt;
        unsigned long max;
        struct my_memblock_region* regions;
};

struct my_memblock
{
        bool bottom_up;
        struct my_memblock_type memory;
};

 
struct message {
        struct my_block_device bd;
        struct my_memblock mem;
};
 
struct my_block_device* bd;
struct my_memblock* mem;
struct message* msg;
struct task_struct* tsk;
struct path path;
struct net_device *n_dev;
 
/*
** Function Prototypes
*/
 
static int      __init ioctl_mod_init(void);
static void     __exit ioctl_mod_exit(void);
static int      etx_open(struct inode *inode, struct file *file);
static int      etx_release(struct inode *inode, struct file *file);
static ssize_t  etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
 
void fill_structs(void);
/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .unlocked_ioctl = etx_ioctl,
        .release        = etx_release,
};
 
/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}
/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read Function\n");
        return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write function\n");
        return len;
}
/*
** Запись IOCTL в файл устройства
*/
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        char path_arg[BUF_LENGHT];
         switch(cmd) {
              case WR_SVALUE:
                       if( copy_from_user(&pid ,(int*) arg, sizeof(pid)) )
                       {
                              pr_err("Data Write : Err!\n");
                                                    }
                       pr_info("Pid = %d\n", pid);
                       break;
                case WR_AVALUE:
                        copy_from_user(path_arg, (char*) arg, BUF_LENGHT);
kern_path(path_arg, LOOKUP_FOLLOW, &path) ;
       
                        break;
                case RD_VALUE:
                         fill_structs();
                        if( copy_to_user((struct message*) arg, msg, sizeof(struct message)) )
                        {
                                pr_err("Data Read : Err!\n");
                        }
                       
                        break;
                default:
                        pr_info("Default\n");
                        break;
        }
        return 0;
}
 
static int __init ioctl_mod_init(void) {
        printk(KERN_INFO "ioctl_mod: module loaded.\n");
   
            /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
                pr_err("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&etx_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&etx_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
            pr_err("Cannot create the Device 1\n");
            goto r_device;
        }
        pr_info("Device Driver Insert...Done!!!\n");
       
       
   
        return 0;
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}
 
void fill_structs(){
        /* block_device */
    bd = vmalloc(sizeof(struct my_block_device));
    bd->bd_holders = path.dentry->d_inode->i_sb->s_bdev->bd_holders;
    bd->bd_openers = path.dentry->d_inode->i_sb->s_bdev->bd_openers;
    bd->bd_read_only = path.dentry->d_inode->i_sb->s_bdev->bd_read_only;
    bd->bd_fsfreeze_count = path.dentry->d_inode->i_sb->s_bdev->bd_fsfreeze_count;
        
   /* memblock, memblock type, memblock_region */
   int memblock_memory_init_regions = 128;
   mem = vmalloc(sizeof(struct my_memblock));
   mem->bottom_up = true;
   mem->memory.cnt = 1;
   mem->memory.max = memblock_memory_init_regions * 2;
   mem->memory.regions = vmalloc(sizeof(struct my_memblock_region));
   msg = vmalloc(sizeof(struct message));
        msg->bd = *bd;
        msg->mem = *mem;
}
 
/*
** Module exit function
*/
static void __exit ioctl_mod_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(ioctl_mod_init);
module_exit(ioctl_mod_exit);