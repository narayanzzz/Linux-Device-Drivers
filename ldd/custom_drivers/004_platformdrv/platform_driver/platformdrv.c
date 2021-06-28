#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/platform_device.h>


#define NO_OF_DEVICES 4

#define RDONLY	1
#define WRONLY	2	
#define RW		3

#define MAX_MEMSIZE_DEV1 1024
#define MAX_MEMSIZE_DEV2 1024
#define MAX_MEMSIZE_DEV3 1024
#define MAX_MEMSIZE_DEV4 1024

char device1_buffer[MAX_MEMSIZE_DEV1];
char device2_buffer[MAX_MEMSIZE_DEV2];
char device3_buffer[MAX_MEMSIZE_DEV3];
char device4_buffer[MAX_MEMSIZE_DEV4];


struct pcdev_private_data
{
	char *buffer;
	unsigned size;
	const char* serial_number;
	int perm;
	struct cdev cdev_d;
};

struct pcdrv_private_data
{
	int total_devices;
	dev_t device_number;
	struct class *class_pcd;
	struct device *device_pcd;
	struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
};


struct pcdrv_private_data pcdrv_data = {
	.total_devices = NO_OF_DEVICES,
	.pcdev_data = {
		[0]={
				.buffer = device1_buffer,
				.size = 1024,
				.serial_number = "pcd dev 1",
				.perm = RDONLY,
			},
		[1]={
				.buffer = device2_buffer,
				.size = 1024,
				.serial_number = "pcd dev 2",
				.perm = WRONLY,
			},
		[2]={
				.buffer = device3_buffer,
				.size = 1024,
				.serial_number = "pcd dev 3",
				.perm = RW,
			},
		[3]={
				.buffer = device4_buffer,
				.size = 1024,
				.serial_number = "pcd dev 4",
				.perm = RW,
			}
	}
};


loff_t pcd_llseek(struct file *filp, loff_t offset, int whence)
{
	loff_t temp;
	struct pcdev_private_data* pcd_dev = (struct pcdev_private_data*)filp->private_data;
	int max_size = pcd_dev->size;

	pr_info("llseek requested\n");

	switch (whence)
	{
	case SEEK_SET:
		if(offset > max_size || offset<0)
		{
			return -EINVAL;
		}

		filp->f_pos = offset;

		break;
	
	case SEEK_CUR:
		temp = filp->f_pos + offset;
		if(temp > max_size || temp <0)
		{
			return -EINVAL;
		}

		filp->f_pos = temp;
		break;
	
	case SEEK_END:
		temp = max_size + offset;

		if(temp > max_size || temp<0)
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

	struct pcdev_private_data* pcd_dev = (struct pcdev_private_data*)filp->private_data;
	int max_size = pcd_dev->size;

	pr_info("Read requested for %zu bytes\n",count);
	pr_info("Current file offset :%lld",*f_pos);

	if(*f_pos + count > max_size)
	{
		count =  max_size - *f_pos;
	}

	if(copy_to_user(user_buffer,pcd_dev->buffer+(*f_pos),count)!=0)
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
	struct pcdev_private_data* pcd_dev = (struct pcdev_private_data*)filp->private_data;
	int max_size = pcd_dev->size;

	pr_info("Write requested for %zu bytes\n",count);
	pr_info("Current file offset :%lld",*f_pos);


	if(*f_pos+count > max_size)
	{
		count = max_size - *f_pos;
		overflow = *f_pos + count - max_size;
	}

	if(copy_from_user(pcd_dev->buffer+(*f_pos),user_buffer,count))
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

int check_permission(unsigned int device_perm,unsigned int acc_mode)
{
	if(device_perm == RW)
	{
		return 0;
	}
	else if(device_perm == RDONLY && ((acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE)) )
	{
		return 0;
	}
	else if(device_perm == WRONLY && ((acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ)) )
	{
		return 0;
	}
	else
	{
		return -EPERM;
	}

}

int pcd_open(struct inode *inode, struct file *filp)
{
	int ret=0xFF;
	struct pcdev_private_data * pcdev_data;

	int minor_n = MINOR(inode->i_rdev);

	pr_info("minor device = %d",minor_n);

	pcdev_data = container_of(inode->i_cdev,struct  pcdev_private_data,cdev_d);
	
	filp->private_data = pcdev_data;

	ret = check_permission(pcdev_data->perm,filp->f_mode);

	!ret?pr_info("Open successfull\n"):pr_err("Open failed\n");

	return ret;
}

int pcd_release(struct inode *inode, struct file *filp)
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

int pcd_platform_probe(struct platform_device *pdev)
{
	pr_info("platform driver probe function called\n");
	return 0;
}

int pcd_platform_remove(struct platform_device *pdev)
{
	pr_info("platform drivers remove function is called\n");
	return 0;
}

struct platform_driver platform_pcd_driver={
	.probe = pcd_platform_probe,
	.remove = pcd_platform_remove,
	.driver = {
		.name = "pseudo-platformdev"
	}
};

static int __init platform_driver_init(void)
{
	platform_driver_register(&platform_pcd_driver);
	pr_info("platform driver is initialized\n");
	return 0;
}

static void __exit platform_driver_cleanup(void)
{
	platform_driver_unregister(&platform_pcd_driver);
	pr_info("platform driver is released\n");
}

module_init(platform_driver_init);
module_exit(platform_driver_cleanup);

// static int __init pcd_init(void)
// {
// 	int i=0;
// 	pr_info("Pseudo Driver Initialization\n");

// 	alloc_chrdev_region(&pcdrv_data.device_number, 0,NO_OF_DEVICES, "pcdevs");

// 	pcdrv_data.class_pcd = class_create(THIS_MODULE, "class_pcd");

// 	for(i=0;i<NO_OF_DEVICES;i++)
// 	{
// 		cdev_init(&pcdrv_data.pcdev_data[i].cdev_d, &pcds_fops);

// 		pcdrv_data.pcdev_data[i].cdev_d.owner = THIS_MODULE;

// 		cdev_add(&pcdrv_data.pcdev_data[i].cdev_d, pcdrv_data.device_number+i, 1);

// 		pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.device_number+i, NULL, "pseudo_device-%d",i+1);
// 	}

// 	pr_info("Module Init Sucessfull\n");

// 	return 0;
// }

// static void __exit pcd_exit(void)
// {
// 	int i=0;
// 	for(i=0;i<NO_OF_DEVICES;i++)
// 	{
// 		device_destroy(pcdrv_data.class_pcd,pcdrv_data.device_number+i);
// 		cdev_del(&pcdrv_data.pcdev_data[i].cdev_d);
		
// 	}

// 	class_destroy(pcdrv_data.class_pcd);

// 	unregister_chrdev_region(pcdrv_data.device_number,4);	


// 	pr_info("PCD Cleanup Sucessfull\n");
// }

// module_init(pcd_init);
// module_exit(pcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Narayan Rathod");
MODULE_DESCRIPTION("Pseudo Driver");
MODULE_INFO(board, "Beaglebone Black Rev C");

