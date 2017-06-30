#include <linux/device.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include "dev.h"

static int major = 0;
module_param(major, int, S_IRUGO);

#define EOK 0

static int device_open = 0;

static int dev_open(struct inode* n, struct file* f){
	if(device_open)
		return -EBUSY;

	++device_open;
	return EOK;
}

static int dev_release(struct inode* n, struct file* f){
	--device_open;
	return EOK;
}

static const struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_release, 
	.read = dev_read,
};

#define DEVICE_FIRST 0		// <---. первый minor
#define DEVICE_COUNT 3		// <---. число поддерживаемых minor
#define MODNAME "my_dyndev_mod"

static struct cdev hcdev;
static struct class* devclass;

static int __init dev_init(void){
	int ret, i;
	dev_t dev;
	if(major != 0){
		dev = MKDEV(major, DEVICE_FIRST);
		ret = register_chrdev_region(dev, DEVICE_COUNT, MODNAME);
	} else{
		ret = alloc_chrdev_region(&dev, DEVICE_FIRST, DEVICE_COUNT, MODNAME);
		major = MAJOR(dev);	// <---. фиксируем номер major
	}

	if(ret < 0){
		printk(KERN_ERR "=== Can not register char device region\n");
		goto err;
	}

	cdev_init(&hcdev, &dev_fops);
	hcdev.owner = THIS_MODULE;
	ret = cdev_add(&hcdev, dev, DEVICE_COUNT);
	if(ret < 0){
		unregister_chrdev_region(MKDEV(major, DEVICE_FIRST), DEVICE_COUNT);
		printk(KERN_ERR "=== Can not add char device\n");
		goto err;
	}
	devclass = class_create(THIS_MODULE, "dyn_class");
	for(i = 0; i < DEVICE_COUNT; ++i){
#define DEVNAME "dyn"
		dev = MKDEV(major, DEVICE_FIRST + i);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,26)
		device_create(devclass, NULL, dev, "%s_%d", DEVNAME, i);
#else
		device_create(devclass, NULL, dev, NULL, "%s_%d", DEVNAME, i);
#endif
	}

	printk(KERN_INFO "===== module installed %d:[%d-%d] =====\n",MAJOR(dev), DEVICE_FIRST, DEVICE_COUNT);

err:
	return ret;
}	

static void __exit dev_exit(void){
	dev_t dev;
	int i;
	for(i = 0; i < DEVICE_COUNT; ++i){
		dev = MKDEV(major, DEVICE_FIRST + i);
		device_destroy(devclass, dev);
	}

	class_destroy(devclass);
	cdev_del(&hcdev);
	unregister_chrdev_region(MKDEV(major, DEVICE_FIRST), DEVICE_COUNT);
	printk(KERN_INFO "===== module removed =====");
}
