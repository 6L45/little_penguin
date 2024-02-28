#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/fs.h>		// file operations structure to  use open/close, read/write to device
#include <linux/cdev.h>		// this is a char driver; make cdev available
#include <linux/semaphore.h>	// semaphore to handle synchronization behavior
//#include <asm/uacess.h>		// copy to user, copy from useR


MODULE_LICENSE("GPL");
MODULE_AUTHOR("mberengu");
MODULE_DESCRIPTION("Hey world!");

// structure for device
struct	fortytwo {
	char			data[100];
	struct semaphore	sem;
}	virtual_device;

// To register the device we need a cdev object and some variables
// we declare variables as globals cause the stack in the kernel is very small
// so not good to declare to much variable inside functions
struct cdev	*ftdev;
int		major_number;
int		ret;
dev_t		dev_num;

#define DEVICE_NAME "fortytwo"

int	device_open(struct inode *node, struct file *filp)
{
	//only allow one process to open this device by using a semaphore
	if (down_interruptible(&virtual_device.sem) != 0)
	{
		printk(KERN_ALERT "fortytwo: could not lock device during open");
		return -1;
	}
	printk(KERN_INFO "fortytwo: opened device");
	return 0;
}

ssize_t	device_read(struct file *filp, char *bufStoreData, size_t bufCount, loff_t *curOffset)
{
	//take data from kernel space(device) to user space (process)
	// copy_to_user (destination, source, sizeToTransfer)
	printk(KERN_INFO "fortytwo: Reading from device");
	ret = copy_to_user(bufStoreData, virtual_device.data, bufCount);
	return ret;
}

ssize_t	device_write(struct file *filp, const char *bufStoreData, size_t bufCount, loff_t *curOffset)
{
	//take data from kernel space(device) to user space (process)
	// copy_from_user (destination, source, sizeToTransfer)
	printk(KERN_INFO "fortytwo: Writing from device");
	ret = copy_from_user(virtual_device.data, bufStoreData, bufCount);
	return ret;
}

int	device_close(struct inode *inode, struct file *filp)
{
	// by calling up (opposite of down for semaphore), we realease the mutex
	up(&virtual_device.sem);
	printk(KERN_INFO "fortytwo: closed device");
	return 0;
}

// telle the kernel which functions to call when user operates on our device file
struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.write = device_write,
	.read = device_read
};

static int	driver_entry(void)
{
	// register our device with the system: 2steps
	// 1 use dynamic allocation to assign our device:
	// 	a major number-- alloc_chrdev_region(dev_t*, uint fminor, uint count, char *name)
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if (ret < 0) // error
	{
		printk(KERN_ALERT "failed to allocate a major number");
		return ret;
	}
	major_number = MAJOR(dev_num);	// extracts the major number and store it
	printk(KERN_INFO "fortytwo driver: %d\n", major_number);
	printk(KERN_INFO "use \"mknod /dev/%s c %d 0\" for device file\n", DEVICE_NAME, major_number); // dmesg
	
	// 2
	ftdev = cdev_alloc(); // create our cdev struct, initialized our cdev
	ftdev->ops = &fops;
	ftdev->owner = THIS_MODULE;

	// now cdev is created, we have to add it to the kernel
	// int cdev_add(struct cdev* dev, dev_t num, unsigned int count)
	ret = cdev_add(ftdev, dev_num, 1);
	if (ret < 0) // error
	{
		printk(KERN_ALERT "fortytwo: unable to add cdev to kernel");
		return ret;
	}

	// 3 initialize semaphore
	sema_init(&virtual_device.sem, 1);
	printk(KERN_INFO "here we are\n"); // dmesg
	return 0;
}

static void	driver_exit(void)
{
	cdev_del(ftdev);
	unregister_chrdev_region(dev_num, 1);
	printk(KERN_ALERT "fortytwo: unload module");
}

module_init(driver_entry);
module_exit(driver_exit);
