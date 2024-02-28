#include <linux/kernel.h>
//#include <linux/uaccess.h>
#include <linux/string.h>

#define LOGIN "mberengu"

static ssize_t	misc_write(struct file *file, const char __user *user_buf,
			   size_t user_len, loff_t *ppos)
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
	int	bytes_to_read;

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
static const struct file_operations id_fops = {
	.read = misc_read,
	.write = misc_write,
};

