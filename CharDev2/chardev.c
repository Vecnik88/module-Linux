/*			Created by Vecnik88 Kernel 4.4.0.79
 *
 *	Создание читающего устройства, но не пишущего, простая практика
 *	После загрузки модуля, проверить сообщения ядра, в них будет указан старший номер устройства
 * 	После необходимо создать это устройство "mknod /dev/chardev c  "старший номер" "младший номер""
 *
 * 	Добавить при сборке ---> make -C /usr/src/linux-headers-`uname -r` SUBDIRS=$PWD modules
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>		// <---. put_user();

// Прототипы функций
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

#define SUCCESS 0
#define DEVICE_NAME "chardev"
#define BUF_LEN 80			// <---. buffer

static int Major;			// <---. старший номер устройства драйвера
static int Device_Open = 0;		// <---. устройство открыто?
static char msg[BUF_LEN];		// <---. текст сообщения
static char* msg_Ptr;

static struct file_operations fops ={
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

int init_module(void){
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if(Major < 0){
		printk("Registering the character device failed with %d\n", Major);
		return Major;
	}

	printk("<1>I was assigned major number %d.  To talk to\n", Major);
  	printk("<1>the driver, create a dev file with\n");
  	printk("'mknod /dev/chardev c %d 0'.\n", Major);
  	printk("<1>Try various minor numbers.  Try to cat and echo to\n");
  	printk("the device file.\n");
 	printk("<1>Remove the device file and module when done.\n");

  	return 0;
}

void cleanup_module(void){
	printk("Hello bro");
	unregister_chrdev(Major, DEVICE_NAME);
}

static int device_open(struct inode* inode, struct file* file){
	static int counter = 0;
	if(Device_Open)
		return -EBUSY;

	++Device_Open;
	sprintf(msg, "I already told you %d times Hello world!\n", ++counter);
	msg_Ptr = msg;	
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

static int device_release(struct inode* inode, struct file* file){
	--Device_Open;

	module_put(THIS_MODULE);

	return 0;
}

static ssize_t device_read(struct file* filp, char* buffer, size_t length, loff_t* offset){
	int bytes_read = 0;

	if(*msg_Ptr == 0)
		return 0;
	
	while(length && *msg_Ptr){
		put_user(*(msg_Ptr++), buffer++);
		--length;
		++bytes_read;
	}

	return bytes_read;
}

static ssize_t device_write(struct file* filp, const char* buffer, size_t length, loff_t* offset){
	printk("<1>Sorry, this operation isn't supported.\n");
	return -EINVAL;
}
