#include <linux/kernel.h>
#include <linux/module.h>

int init_module(void){
	printk("Hello world - this is the kernel speaking");
	return 0;
}
