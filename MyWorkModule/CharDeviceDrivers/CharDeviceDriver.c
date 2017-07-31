/* Разобраться с методами poll and ioctl */

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/version.h>

/* Описание модуля */
MODULE_VERSION( "1.0" );
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Vecnik88" );
MODULE_DESCRIPTION( "Символьное устройство" );

/* макросы для удобства работы с printk */
#define ERR(...) printk( KERN_ERR __VA_ARGS__ )
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )
#define DEBUG(...) if( debug ) printk( KERN_DEBUG "DEBUG MESSAGE " __VA_ARGS__ )

#define DEVNAME "symbol_driver_test"			/* имя драйвера */
#define MODNAME "my_symbol_driver_module"		/* имя модуля */

#define BUF_SIZE 8192							/* размер буфера */
#define DEVICE_FIRST 0							/* первый номер минор */
#define DEVICE_COUNT 3							/* число поддерживаеых минор номеров */

/* Макросы для ioctl */
#define CHAR_IOC_MAGIC 'k'

#define CHAR_IOCRESET _IO( CHAR_IOC_MAGIC, 0 )
#define CHAR_IOC_S_UANTUM _IOW( CHAR_IOC_MAGIC, 1, int )
#define CHAR_IOC_T_QUANTUM _IO( CHAR_IOC_MAGIC, 2 )
#define CHAR_IOC_G_QUANTUM _IOR( CHAR_IOC_MAGIC, 3, int )
#define CHAR_IOC_MAXNR 5

/* Параметры, которые можно задать при загрузке модуля */

static int major = 0;							// <---. старший номер устройства
module_param( major, int, S_IRUGO );

static int debug = 0;							// <---. для отладки модуля, если задать 1 то будут выводиться отладочные сообщения
module_param( debug, int, 0 );

static int dev_open_count = 0;

static char my_buffer[ BUF_SIZE ] = " Hello world\n ";

static ssize_t dev_read( struct file* file, char* buf, size_t count, loff_t* ppos ){
	int res = 0;

	if( *ppos > sizeof( my_buffer ) ){
		ERR( "Error memory" );
		return -ENOMEM;
	}

	if( ( count + *ppos) > strlen( my_buffer ) )
		count = strlen( my_buffer );

	copy_to_user( ( void* )buf, my_buffer + *ppos, count );

	put_user( '0', buf + count );

	res = count + 1;

	*ppos += count;

	DEBUG( "===== Read %d bytes =====\n", res );

	return res;
}

static ssize_t dev_write( struct file* file, const char* buf, size_t count, loff_t* ppos ){
	int res = 0;

	if( count + *ppos > sizeof( my_buffer ) )
		return -EINVAL;

	copy_from_user( my_buffer + *ppos, ( void* )buf, count );

	DEBUG( "===== dev_write ---> res = %d =====\n", res );

	my_buffer[ count ] = '\0';

	res = count + 1;
	*ppos += count;

	return res;
}

static int dev_open( struct inode* n, struct file* f ){
	if( dev_open_count != 0 )
		return -EBUSY;

	++dev_open_count;

	LOG( "===== dev_open work =====\n" );

	return 0;
}

static int dev_close( struct inode* n, struct file* f ){
	LOG( "===== dev_close work =====\n" );

	--dev_open_count;

	return 0;
}

/*#if LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,35 )
static long dev_ioctl( struct file* f, unsigned int cmd, unsigned long arg ){
#else
static int dev_ioctl( struct inode* n, struct file* f, unsigned int cmd, unsigned long arg ){
#endif
	if( ( _IOC_TYPE( cmd ) ) != IOC_MAGIC )
		return -EINVAL;

	switch( cmd ){
		case IOCTL_GET_STRING:
			if( copy_to_user( ( void* )arg, hello_str, _IOC_SIZE( cmd ) ) )
				return -EFAULT;
				break;

		default:
			return -ENOTTY;
	}

	return 0;
}*/

static const struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_close,
	.read = dev_read,
	.write = dev_write,/*
#if LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,35 )
	.unlocked_ioctl = dev_ioctl,
#else
	.ioctl = dev_ioctl,
#endif*/
};

static struct cdev hcdev;
static struct class* devclass;

static int __init dev_init( void ){
	int ret = 0;							/* для ошибок */
	int i = 0;								/* счетчик в циклах */
	dev_t dev;								/* старший и младший номера устройств */

	if( major == 0 ){
		ret = alloc_chrdev_region( &dev, DEVICE_FIRST, DEVICE_COUNT, MODNAME );
		major = MAJOR( dev );					// <---. фиксируем старший номер нашего устройства
	} else {
		dev = MKDEV( major, DEVICE_FIRST );
		ret = register_chrdev_region( dev, DEVICE_COUNT, MODNAME );
	}

	if( ret < 0 ){
		ERR( "===== Symbol Device is not register =====\n" );
		goto err;
	}

	cdev_init( &hcdev, &dev_fops );
	hcdev.owner = THIS_MODULE;
	ret = cdev_add( &hcdev, dev, DEVICE_COUNT );
	if( ret < 0 ){
		unregister_chrdev_region( MKDEV( major, DEVICE_FIRST ), DEVICE_COUNT );
		ERR( "===== Symbol Device is not ADD =====\n" );
		goto err;
	}

	devclass = class_create( THIS_MODULE, "dyn_class" );
	for( i = 0; i < DEVICE_COUNT; ++i ){
		dev = MKDEV( major, DEVICE_FIRST + i );
#if LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,26 )
		device_create( devclass, NULL, dev, "%s_%d", DEVNAME, i );
#else	
		/* 					!!!!! ВАЖНО !!!!!

			прототип device_create изменился после 2,6,26 - 
			код компилироваться будет без ошибок, но работать не будет 
		*/
		device_create( devclass, NULL, dev, NULL, "%s_%d", DEVNAME, i );
#endif
	}

	LOG( "===== MODULE DEV_INIT INSTALLED %d:[ %d - %d ] =====\n",
		MAJOR( dev ), DEVICE_FIRST, MINOR( dev ) );

err:
	return ret;
}

static void __exit dev_exit( void ){
	dev_t dev;
	int i = 0;
	for( i = 0; i < DEVICE_COUNT; ++i ){
		dev = MKDEV( major, DEVICE_FIRST + i );
		device_destroy( devclass, dev );
	}
	class_destroy( devclass );
	cdev_del( &hcdev );
	unregister_chrdev_region( MKDEV( major, DEVICE_FIRST ), DEVICE_COUNT );

	LOG( "===== MODULE DEV_EXIT REMOVED =====\n" );
}

module_init( dev_init );
module_exit( dev_exit );
