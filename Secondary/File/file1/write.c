							/* Работа с vfs in KERNEL LINUX :) */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define BUF_LEN 255
#define DEFLOG "./module.log"
#define TEXT "Hello bro, is a kernel\n"

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static char* log = NULL;
module_param( log, charp, 0 );

static struct file* f;

static int __init init_write( void ){
	ssize_t n = 0;
	loff_t offset = 0;
	mm_segment_t fs;
	char buff[ BUF_LEN + 1 ] = DEFLOG;
	if( log != NULL )
		strcpy( buff, log );
	f = filp_open( buff, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR );
	if( IS_ERR( f ) ){
		LOG( "! file open failed: %s\n", buff );
		return -ENOENT;
	}

	LOG( "! file open %s\n", buff );
	fs = get_fs();						// <---. сохраняем для восстановления контекста пользователя в последствии
	set_fs( get_ds() );					// <---. чтобы работать с vfs нужно перейти в пространство ядра
	strcpy( buff, TEXT );

	if( ( n = vfs_write( f, buff, strlen(buff), &offset ) ) != strlen( buff ) ){
		LOG( "! failed to write: %d\n", n );
		return -EIO;
	}

	LOG( "! write %d bytes\n", n );
	LOG( "! %s", buff );

	set_fs( fs );						// <---. восстанавливаем пространство пользователя
	filp_close( f, NULL );
	LOG( "! file_close\n" );

	return -1;
}

module_init( init_write );
MODULE_LICENSE( "GPL" );