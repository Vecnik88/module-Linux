							/* Работа с файлами в ядре */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>

static char* file = NULL;
module_param( file, charp, 0 );

#define BUF_LEN 255
#define DEFNAME "/etc/yumex.profiles.conf"

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static char buff[ BUF_LEN + 1 ] = DEFNAME;

static int __init kread_init( void ){
	struct file* f;
	size_t n;
	long l;
	loff_t file_offset = 0;

	mm_segment_t fs = get_fs();
	set_fs( get_ds() );

	if( file != NULL )
		strcpy( buff, file );

	LOG( "*** openning file: %s\n", buff );
	f = filp_open( buff, O_RDONLY, 0 );

	if( IS_ERR( f ) ){
		LOG( "*** file open failed %s\n", buff );
		l = - ENOENT;
		goto fail_oupen;
	}

	l = vfs_llseek( f, 0L, 2 );
	if( l <= 0 ){
		LOG( "*** failed to lseek %s\n", buff );
		l = -EINVAL;
		goto failure;
	}

	LOG( "*** file size = %d bytes\n", ( int )l );

	vfs_llseek( f, 0L, 0 );
	if( ( n = vfs_read( f, buff, l, &file_offset ) ) != l ){
		LOG( "*** failed to read\n" );
		l = -EIO;
		goto failure;
	}
	buff[ n ] = '\0';
	LOG( "%s\n", buff );
	LOG( "**** close file\n" );
	l = -EPERM;

failure:
	filp_close( f, NULL );
fail_oupen:
	set_fs( fs );
	return ( int )l;
}

module_init( kread_init );
MODULE_LICENSE( "GPL" );