// SPDX-License-Identifier: UNLICENSED

#include <linux/module.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/string.h>

#define LOGIN "mberengu"
int	bytes_to_read;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("mberengu");
MODULE_DESCRIPTION("short misc device for training!");

static int	misc_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "fortytwo: Open called\n");
	printk(KERN_INFO "\tfortytwo: Device Numbers: %d - %d\n", imajor(inode), iminor(inode));

	// file usage exemples
/*	if (file->f_mode & FMODE_READ)
		printk(KERN_INFO "fortytwo: called with read permissions\n");
	if (file->f_mode & FMODE_READ)
		printk(KERN_INFO "fortytwo: called with write permissions\n");*/
	return 0;
}

static int	misc_close(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "fortytwo: Close called\n");
	return 0;
}

static ssize_t	misc_write(struct file *file, const char __user *user_buf, size_t user_len, loff_t *ppos)
{
	char	*kernel_buf;

	printk(KERN_INFO "fortytwo: misc_write called\n");

	// can't get user_buf directly. Have to pass through copy_from_user
	// so creation of a variable to handle the reception of the buffer
	kernel_buf = kmalloc(user_len + 1, GFP_KERNEL);
	kernel_buf[user_len] = 0;
	if (!kernel_buf)
		return -ENOMEM;

	// copy from user = copy from user space to here.
	if (copy_from_user(kernel_buf, user_buf, user_len)) {
		kfree(kernel_buf);
		return -EFAULT;
	}
	if (strcmp(LOGIN, kernel_buf)) {
		kfree(kernel_buf);
		return -EINVAL;
	}
	kfree(kernel_buf);
	return user_len;
}

static ssize_t	misc_read(struct file *file, char __user *user_buf, size_t user_len, loff_t *ppos)
{
	int	status;

	// exit hadndler (reading operations need a 0 as for eof)
	if (*ppos >= 8)
		return 0;

	printk(KERN_INFO "fortytwo: misc_read called\n");

	// copy to user = copy to userspace (so from here to userspace)
	status = copy_to_user(user_buf, LOGIN, 8);
	if (status) {
		printk(KERN_ALERT "fortytwo: Err during copy_to_user\n");
		return -status;
	}

	// update ppos position
	bytes_to_read = (user_len < 8 - *ppos) ? user_len : 8 - *ppos; // b_t_r = min()
	*ppos = *ppos + bytes_to_read;
	return 8;
}

// events function call
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = misc_read,
	.write = misc_write,
	.open = misc_open,
	.release = misc_close
};

// define device
static struct miscdevice miscdev = {
	.name = "fortytwo",
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops
};

// register
static	int __init misc_init(void)
{
	int status;

	printk(KERN_INFO "fortytwo: Hello %s\n", LOGIN);
	status = misc_register(&miscdev);
	if (status) {
		printk(KERN_ALERT "fortytwo: could not register device");
		return -EINVAL;
	}
	return 0;
}

//deregister
static void __exit misc_exit(void)
{
	printk(KERN_INFO "See ya!\n");
	misc_deregister(&miscdev);
}

module_init(misc_init);
module_exit(misc_exit);
