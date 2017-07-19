#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/uaccess.h>

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static unsigned long waddr = 0;
static char buf[ 80 ];
static int len;

int symb_fn( void* data, const char* sym, struct module* mod, unsigned long addr ){
	if( strcmp( ( char* )data, sym ) == 0 ){
		waddr = addr;
		return 1;
	} else{
		return 0;
	}
}

/* asmlinkage long sys_write( unsigned int fd, const char __user* buf, size_t count ) */

static asmlinkage long( *sys_write )(
	unsigned int fd, const char __user* buf, size_t count);

static int do_write( void ){
	int n;
	mm_segment_t fs = get_fs();
	set_fs( get_ds() );
	sys_write = ( void* )waddr;
	n = sys_write( 1, buf, len );
	set_fs( fs );

	return n;
}

static int __init wr_init( void ){
	int n = kallsyms_on_each_symbol( symb_fn, ( void* )"sys_write" );

	if( n != 0 ){
		sprintf( buf, "address system req = %lx\n", waddr );
		len = strlen( buf ) + 1;
		LOG( "+ [%d]: %s", len, buf );
		n = do_write();
		LOG( "+ write return: %d\n", n );
	} else {
		LOG( "+ %d: symbol not found\n", n );
	}

	return -EPERM;
}

module_init( wr_init );
MODULE_LICENSE( "GPL" );










