#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>

static char* fileName = NULL;
module_param( fileName, charp, 0 );

#define BUF_LEN 255
#define DEFNAME "/etc/yumex.profiles.conf"

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static char buff[ BUF_LEN + 1 ] = DEFNAME;

static int __init kread_init( void ){
	struct file* f;
	size_t n;
	if( fileName != NULL )
		strcpy( buff, fileName );

	LOG( "*** openning file: %s\n", buff );
	f = filp_open( buff, O_RDONLY, 0 );
	if( IS_ERR( f ) ){
		LOG( "*** file open failed: %s\n", buff );
		return -ENOENT;
	}

	n = kernel_read( f, 0, buff, BUF_LEN );

	if( n ){
		LOG( "*** read first %d bytes:\n", n );
		buff[ n ] = '\0';
		LOG( "%s\n", buff );
	} else {
		LOG( "*** kernel_read failed\n" );
		return -EIO;
	}

	LOG( "*** close file\n" );
	filp_close( f, NULL );
	return - EPERM;
}

module_init( kread_init );
MODULE_LICENSE( "GPL" );