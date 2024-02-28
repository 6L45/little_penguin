// SPDX-License-Identifier: UNLICENSED

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("mberengu");

 /* That was a long sleep, tell userspace about it */
static int do_work(int *my_int)
{
	int x;
	int y = *my_int;
	int z;

	for (x = 0; x < y; ++x)
		usleep_range(8, 12);
	if (y < 10)
		pr_info("We slept a long time!");
	z = x * y;
	return z;
}

static int __init my_init(void)
{
	int x = 10;

	do_work(&x);
	return 0;
}

static void __exit my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);
