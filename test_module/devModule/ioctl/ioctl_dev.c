#include <linux/version.h>
#include "ioctl.h"
#include "../dev.h"

static int dev_open(struct inode* n, struct file* f){
	return 0;
}

static int dev_release(struct inode* n, struct file* f){
	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
static long dev_ioctl(struct file* f, unsigned int cmd, unsigned long arg){
#else
static int dev_ioctl(struct inode* n, struct file* f,
		     unsigned int cmd, unsigned long arg){
#endif
	if((_IOC_TYPE(cmd) != IOC_MAGIC))
		return -ENOTTY;

	switch(cmd){
		case IOCTL_GET_STRING:
			if(copy_to_user((void*)arg, hello_str, _IOC_SIZE(cmd)))
				return -EFAULT;
			break;
		default:
			return -ENOTTY;
	}

	return 0;
}

static const struct file_operations hello_fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_release,
	.read = dev_read,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	.unlocked_ioctl = dev_ioctl
#else
	.ioctl = dev_ioctl
#endif
};

#define HELLO_MAJOR 200
#define HELLO_MODNAME "my_ioctl_dev"

static int __init dev_init(void){
	int ret = register_chrdev(HELLO_MAJOR, HELLO_MODNAME, &hello_fops);
	if(ret < 0){
		printk(KERN_ERR "========== Can not register char device ==========\n");
		goto err;
	}

err:
	return ret;
}

static void __exit dev_exit(void){
	unregister_chrdev(HELLO_MAJOR, HELLO_MODNAME);
}
