#include <linux/fs.h>
#include <linux/miscdevice.h>
#include "../dev.h"

static int minor = 0;
module_param(minor, int, S_IRUGO);

static const struct file_operations misc_fops = {
	.owner = THIS_MODULE,
	.read = dev_read,
};

static struct miscdevice misc_dev = {
	MISC_DYNAMIC_MINOR,
	"own_misc_dev",
	&misc_fops
};

static int __init dev_init(void){
	int ret;
	if(minor != 0)
		misc_dev.minor = minor;

	ret = misc_register(&misc_dev);
	if(ret)	
		printk(KERN_ERR "=== Unable to register misc device\n");
		
	return ret;
}

static void __exit dev_exit(void){
	misc_deregister(&misc_dev);
}
