#include <linux/kernel.h>
#include <linux/uaccess.h>

static size_t 		data_size = 0;
static char 		data[PAGE_SIZE];
static struct mutex	foo_mutex;

static ssize_t foo_read(struct file *file, char *buffer, size_t len, loff_t *ppos)
{
	int	ret;
	int	bytes_to_read;

	if (*ppos >= data_size)
		return 0;

	printk(KERN_INFO "debugfs: foo_read called\n");
	ret = mutex_lock_interruptible(&foo_mutex);
	if (ret)
		return (ret);

	bytes_to_read = len < (data_size - *ppos) ? len : (data_size - *ppos); // b_t_r = min()
	ret = copy_to_user(buffer, data, bytes_to_read);
	if (ret) {
		printk(KERN_ALERT "debugfs: Err during copy_to_user\n");
		mutex_unlock(&foo_mutex);
		return -ret;
	}
	*ppos = *ppos + bytes_to_read;
	mutex_unlock(&foo_mutex);
	return bytes_to_read;
}

static ssize_t foo_write(struct file *file, const char *buffer, size_t len, loff_t *ppos)
{
	int	ret;

	if (*ppos + len >= PAGE_SIZE)
		return (-ENOSPC);

	ret = mutex_lock_interruptible(&foo_mutex);
	if (ret)
		return ret;

	ret = copy_from_user(data, buffer, len);
	if (ret) {
		mutex_unlock(&foo_mutex);
		return (-EINVAL);
	}
	mutex_unlock(&foo_mutex);
	*ppos += len;
	data_size = len;
	return len;
}

static struct file_operations foo_fops = {
  .read = foo_read,
  .write = foo_write,
};
