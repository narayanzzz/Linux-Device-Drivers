#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include<linux/uaccess.h>


#define BUF_SIZE 512

char deviceBuffer[BUF_SIZE];

dev_t device_number;
struct cdev pcd_cdev;

struct class *pcd_class;
struct device *device;

loff_t pcd_llseek(struct file *filp, loff_t offset, int whence)
{
	loff_t temp;

	pr_info("llseek requested\n");

	switch (whence)
	{
	case SEEK_SET:
		if(offset > BUF_SIZE || offset<0)
		{
			return -EINVAL;
		}

		filp->f_pos = offset;

		break;
	
	case SEEK_CUR:
		temp = filp->f_pos + offset;
		if(temp > BUF_SIZE || temp <0)
		{
			return -EINVAL;
		}

		filp->f_pos = temp;
		break;
	
	case SEEK_END:
		temp = BUF_SIZE + offset;

		if(temp > BUF_SIZE || temp<0)
		{
			return -EINVAL;
		}

		filp->f_pos = temp;

		break;
	
	default:
		return -EINVAL;
	}
	return 0;
}

ssize_t pcd_read(struct file *filp, char __user *user_buffer, size_t count, loff_t *f_pos)
{
	pr_info("Read requested for %zu bytes\n",count);
	pr_info("Current file offset :%lld",*f_pos);

	if(*f_pos + count > BUF_SIZE)
	{
		count =  BUF_SIZE - *f_pos;
	}

	if(copy_to_user(user_buffer,&deviceBuffer[*f_pos],count)!=0)
	{
		return -EFAULT;
	}

	*f_pos += count;

	pr_info("Number of bytes sucessfully read : %zu",count);
	pr_info("Updated file offset :%lld",*f_pos);

	return count;
}

ssize_t pcd_write(struct file *filp, const char __user *user_buffer, size_t count, loff_t * f_pos)
{
	int overflow=0;

	pr_info("Write requested for %zu bytes\n",count);
	pr_info("Current file offset :%lld",*f_pos);

	if(*f_pos+count > BUF_SIZE)
	{
		count = BUF_SIZE - *f_pos;
		overflow = *f_pos + count - BUF_SIZE;
	}

	if(copy_from_user(&deviceBuffer[*f_pos],user_buffer,count))
	{
		return -EFAULT;
	}

	*f_pos += count;

	if(overflow)
	{
		pr_info("No memory for last %d bytes\n",overflow);
		return -ENOMEM;
	}

	pr_info("Number of bytes sucessfully written : %zu",count);
	pr_info("Updated file offset :%lld",*f_pos);
	
	return count;
}

int pcd_open(struct inode *pinode, struct file *filp)
{
	pr_info("Open successfull\n");
	return 0;
}

int pcd_release(struct inode *pinode, struct file *filp)
{
	pr_info("Release sucessfull\n");
	return 0;
}

struct file_operations pcds_fops={
	.open		= pcd_open,
	.llseek 	= pcd_llseek,
	.release 	= pcd_release,
	.read 		= pcd_read,
	.write 		= pcd_write
};

static int __init pcd_init(void)
{
	pr_info("Pseudo Driver Initialization\n");

	alloc_chrdev_region(&device_number, 0, 1, "pcd");

	cdev_init(&pcd_cdev, &pcds_fops);

	pcd_cdev.owner = THIS_MODULE;

	cdev_add(&pcd_cdev, device_number, 1);

	pcd_class = class_create(THIS_MODULE, "class_pcd");

	device = device_create(pcd_class, NULL, device_number, NULL, "pseudo_device");

	return 0;
}

static void __exit pcd_exit(void)
{
	device_destroy(pcd_class,device_number);
	class_destroy(pcd_class);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number,1);

	pr_info("PCD Cleanup Sucessfull\n");
}

module_init(pcd_init);
module_exit(pcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Narayan Rathod");
MODULE_DESCRIPTION("Pseudo Driver");
MODULE_INFO(board, "Beaglebone Black Rev C");
