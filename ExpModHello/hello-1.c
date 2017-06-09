/*
 *	Простейший модуль ядра
 */

#include <linux/module.h>
#include <linux/kernel.h>	// <---. this KERN_ALERT

int init_module(){
	printk("<1>Hello world 1.\n");

	return 0;
}

void cleanup_module(){
	printk(KERN_ALERT "Goodbye world 1.\n");
}
