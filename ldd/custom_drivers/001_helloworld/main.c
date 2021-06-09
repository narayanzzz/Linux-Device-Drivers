#include<linux/module.h>

static int __init helloworld_init(void)
{
	pr_info("Helloworld Init\n");
	return 0;
}

static void __exit helloworld_cleanup(void)
{
	pr_info("Helloworld Cleanup");
}

module_init(helloworld_init);
module_exit(helloworld_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Narayan");
MODULE_DESCRIPTION("A simple hello world kernel module");
MODULE_INFO(board,"Beaglebone Black Rev C");
