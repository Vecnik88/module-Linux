#include <linux/kernel.h>
#include <linux/module.h>

void cleanup_module(void){
	printk("<1> Short is the life of a kernel module\n");
}
