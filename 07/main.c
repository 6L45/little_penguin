#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/jiffies.h>
#include "id.c"
#include "foo.c"

#define LOGIN "mberengu"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(LOGIN);
MODULE_DESCRIPTION("littl_pinguin ex07");

static struct dentry *debugfs_root;

#if BITS_PER_LONG == 32
	#define debugfs_create_long(name, mode, parent, value)\
	debugfs_create_u32(name, mode, parent, (u32 *)(value))
#else
	#define debugfs_create_long(name, mode, parent, value)\
	debugfs_create_u64(name, mode, parent, (u64 *)(value))
#endif

static int __init hello_init(void)
{
	printk(KERN_INFO "Hello World!\n");
	debugfs_root = debugfs_create_dir("fourtytwo", NULL);
	if (!debugfs_root)
		return -ENOENT;
	mutex_init(&foo_mutex);
	if (!debugfs_create_file("id", 0666, debugfs_root, NULL, &id_fops) ||
			!debugfs_create_file("foo", 0644, debugfs_root, NULL, &foo_fops))
		return -ENOENT;
	debugfs_create_long("jiffies", 0444, debugfs_root, &jiffies);
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "Cleaning up module.\n");
	debugfs_remove_recursive(debugfs_root);
}

module_init(hello_init);
module_exit(hello_exit);
