#include<linux/module.h>
#include<linux/platform_device.h>
#include "platform.h"



void pcdev_release(struct device* pdev)
{
    pr_info("pcdev released\n");
}

struct pcdev_data pcdev_platform_data[2]={
    [0] = {
        .serial = "pcdev_pf_1",
        .pem = RW,
        .size = 512
    },

    [1] = {
        .serial = "pcdev_pf_2",
        .pem = RW,
        .size = 512
    }
};

struct platform_device platform_dev_1 = {
    .name = "pseudo-platformdev",
    .id = 0,
    .dev = {
        .platform_data = &pcdev_platform_data[0],
        .release = pcdev_release
    }
};

struct platform_device platform_dev_2 = {
    .name = "pseudo-platformdev",
    .id = 1,
    .dev = {
        .platform_data = &pcdev_platform_data[1],
        .release = pcdev_release
    }
};

static int __init platform_dev_init(void)
{
    platform_device_register(&platform_dev_1);
    platform_device_register(&platform_dev_2);

    pr_info("plaftorm device initialized\n");

    return 0;
}

static void __exit platform_dev_cleanup(void)
{
    platform_device_unregister(&platform_dev_1);
    platform_device_unregister(&platform_dev_2);

    pr_info("plaftorm device released\n");
}

module_init(platform_dev_init);
module_exit(platform_dev_cleanup);

MODULE_AUTHOR("Narayan Rathod");
MODULE_DESCRIPTION("Pseudo Platform device");
MODULE_LICENSE("GPL");