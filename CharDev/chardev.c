/*
 *	Символьное устройство, доступное только для чтения.
 * 	Возвращает сообщение с указ-нием кол-ва произведенных попыток чтения
 *	из файла.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

#define SUCCESS 0
#define DEVICE_NAME "chardev"		// <---. имя устройства, отображается в /proc/devices
#define BUF_LEN 80			// <---. максимальная длина сообщения

static int Major;			// <---. старший номер устройства нашего драйвера
static int Device_Open = 0;		// <---. устройство открыто?
static char msg[BUF_LEN];		// <---. text message
static char* msg_Ptr;

static struct file_operations fops = {
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
	// off device

	/*int ret =*/ unregister_chrdev(Major, DEVICE_NAME);
	/*if(ret < 0)
		printk("Error in unregister_chrdev: %d\n", ret);*/
}

	// Обработчики

static int device_open(struct inode* inode, struct file* file){
	static int counter = 0;
	if(Device_Open)
		return -EBUSY;
	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

static int device_release(struct inode* inode, struct file* file){
	Device_Open--;
	module_put(THIS_MODULE);

	return 0;
}

static ssize_t device_read(struct file *filp, /* см. include/linux/fs.h   */
         char *buffer,                        /* буфер, куда надо положить данные */
         size_t length,                       /* размер буфера */
         loff_t * offset)
{
  /*
   * Количество байт, фактически записанных в буфер
   */
  int bytes_read = 0;

  /*
   * Если достигли конца сообщения, 
   * вернуть 0, как признак конца файла
   */
  if (*msg_Ptr == 0)
    return 0;

  /* 
   * Перемещение данных в буфер
   */
  while (length && *msg_Ptr) {

    /* 
     * Буфер находится в пространстве пользователя (в сегменте данных), 
     * а не в пространстве ядра, поэтому простое присваивание здесь недопустимо. 
     * Для того, чтобы скопировать данные, мы используем функцию put_user, 
     * которая перенесет данные из пространства ядра в пространство пользователя. 
     */
    put_user(*(msg_Ptr++), buffer++);

    length--;
    bytes_read++;
        }

  /* 
   * В большинстве своем, функции чтения возвращают количество байт, записанных в буфер.
   */
  return bytes_read;
}

/*  
 * Вызывается, когда процесс пытается записать в устройство, 
 * например так: echo "hi" > /dev/chardev
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
  printk("<1>Sorry, this operation isn't supported.\n");
  return -EINVAL;
}
