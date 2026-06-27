#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/uaccess.h>

#define BUF_SIZE 100
static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;
static char kernel_buffer[BUF_SIZE]="Hello from kernel";

static int my_open(struct inode *inode,struct file *file)
{
	pr_info("Embedded Device loaded\n");
	return 0;
}

static int my_release(struct inode *inode,struct file *file)
{
	pr_info("Embedded Device close\n");
	return 0;
}

static ssize_t my_read(struct file *file,char __user *buf,size_t len,loff_t *off)
{
	int bytes=strlen(kernel_buffer);
	if(*off>=bytes)
	{
		return 0;	
	}
	if(copy_to_user(buf,kernel_buffer,bytes))
		return -EFAULT;
	*off+=bytes;
	pr_info("Embedded Read called\n");
	return bytes;
}

static ssize_t my_write(struct file *file,const char __user *buf,size_t len,loff_t *off)
{
	if(len>BUF_SIZE-1)
	{
		len=BUF_SIZE-1;
	}
	if(copy_from_user(kernel_buffer,buf,len))
		return -EFAULT;
	kernel_buffer[len]='\0';
	pr_info("Embedded write called\n");
	return len;
}

static struct file_operations fops ={
	.owner=THIS_MODULE,
	.open=my_open,
	.read=my_read,
	.write=my_write,
	.release=my_release
};

static int __init demo_init(void)
{
	int ret;
	ret=alloc_chrdev_region(&dev_num,0,1,"embedded");
	if(ret)
		return ret;
	cdev_init(&my_cdev,&fops);
	ret=cdev_add(&my_cdev,dev_num,1);
	my_class=class_create("dinesh");
	device_create(my_class,NULL,dev_num,NULL,"Embedded");
	pr_info("Driver loaded");
	pr_info("Major=%d Minor=%d\n",MAJOR(dev_num),MINOR(dev_num));
	return 0;
}

static void __exit demo_exit(void)
{
	device_destroy(my_class,dev_num);
	class_destroy(my_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(dev_num,1);
	pr_info("Driver unloaded\n");	
}


module_init(demo_init);
module_exit(demo_exit);
MODULE_LICENSE("GPL");


