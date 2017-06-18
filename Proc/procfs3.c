/*
 *			Created by Vecnik88 Kernel 4.4.0
 *	Создает файл в директории /proc доступный дял записи и чтения
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

int len, temp;

char* msg;

ssize_t read_proc(struct file* filp, char* buf, size_t count, loff_t* offp){
	if(count > temp){
		count = temp;
	}

	temp = temp-count;
	copy_to_user(buf, msg, count);

	if(count == 0){
		temp = len;
	}

	return count;
}

ssize_t write_proc(struct file* filp, const char* buf, size_t count, loff_t* offp){
	copy_from_user(msg, buf, count);
	len = count;
	temp = len;

	return count;
}

struct file_operations proc_fops = {
	.read = read_proc,
	.write = write_proc
};

void create_new_proc_entry(void){
	proc_create("test", 0, NULL, &proc_fops);
	msg = kmalloc(10*sizeof(char), GFP_KERNEL);
}

static int proc_init(void){

	create_new_proc_entry();

	return 0;
}

void proc_cleanup(void){
	kfree(msg);
	remove_proc_entry("test", NULL);
}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);
