// SPDX-License-Identifier: UNLICENSED

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("mberengu");
MODULE_DESCRIPTION("Useless module");

static ssize_t myfd_read(struct file *fp, char __user *user, size_t size, loff_t *offs);
static ssize_t myfd_write(struct file *fp, const char __user *user, size_t size, loff_t *offs);

static const struct file_operations myfd_fops = {
	.owner = THIS_MODULE,
	.read = myfd_read,
	.write = myfd_write
};

static struct miscdevice myfd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "reverse",
	.fops = &myfd_fops
};

char	str[PAGE_SIZE];

static int __init myfd_init(void)
{
	str[0] = 0;
	return misc_register(&myfd_device);
}

static void __exit myfd_cleanup(void)
{
	misc_deregister(&myfd_device);
}

static ssize_t myfd_read(struct file *fp, char __user *user, size_t size, loff_t *offs)
{
	int     t, i;
	int     status;
	char    *tmp;

	tmp = kmalloc(sizeof(char) * size + 1, GFP_KERNEL);
	for (t = strlen(str) - 1, i = 0; t >= 0; t--, i++)
		tmp[i] = str[t];
	tmp[i] = 0x0;

	status = simple_read_from_buffer(user, size, offs, tmp, i);
	kfree(tmp);
	return status;
}

static ssize_t myfd_write(struct file *fp, const char __user *user, size_t size, loff_t *offs)
{
	ssize_t res;

	if (size >= PAGE_SIZE)
		return (-ENOSPC);

	res = simple_write_to_buffer(str, ARRAY_SIZE(str) - 1, offs, user, size);
	if (res >= 0)
		str[res] = 0x0;
	return res;
}

module_init(myfd_init);
module_exit(myfd_cleanup)
