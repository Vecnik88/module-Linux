/* Создание файлов в sys/class */

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/parport.h>
#include <asm/uaccess.h>
#include <linux/pci.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
								Обязательно указываеть лицензию GPL 
		иначе ошибка при созданиях классов будет и модуль скомпилируется, но не загрузится
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
MODULE_LICENSE( "GPL" );

#define LEN_MSG 160
static char buf_msg[ LEN_MSG + 1 ] = "Hello from module!\n";

//#if LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,32 )
static ssize_t x_show( struct class* class, struct class_attribute* attr, char* buf ){
/*#else
static ssize_t x_show(  struct class* class, char* buf ){
#endif*/
	strcpy( buf, buf_msg );
	printk( "read %d\n", strlen( buf ) );
	return strlen( buf );
}

//#if LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,32 )
static ssize_t x_store( struct class* class, struct class_attribute* attr, const char* buf, size_t count ){
/*#else
static ssize_t x_store( struct class* class, const char* buf, size_t count ){
#endif*/
	printk( "write %d\n", count );
	strncpy( buf_msg, buf, count );
	buf_msg[ count ] = '\0';

	return count;
}

CLASS_ATTR( xxx, ( S_IWUSR | S_IRUGO ), &x_show, &x_store );

static struct class* x_class;

static int __init x_init( void ){
	int res;
	x_class = class_create( THIS_MODULE, "x-class" );
	if( IS_ERR( x_class ) )
		printk( "bad class create\n" );

	res = class_create_file( x_class, &class_attr_xxx );
	printk(KERN_INFO "===== MODULE XXX INIT =====\n");

	return 0;
}

static void __exit x_cleanup( void ){
	class_remove_file( x_class, &class_attr_xxx );
	class_destroy( x_class );
	printk(KERN_INFO "===== MODULE XXX REMOVED =====\n");
}

module_init( x_init );
module_exit( x_cleanup );