/* Запуск новых процессов в ядре */

#include <linux/delay.h>
#include <linux/module.h>

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static char* str;
module_param( str, charp, S_IRUGO );

static int __init exec_init( void ){
	int rc;
	char *argv[] = { "wall", "\nthis is wall message ", NULL },
		 *envp[] = { NULL },
		 msg[ 80 ];

	if( str ){
		sprintf( msg, "\n%s", str );
		argv[ 1 ] = msg;
	}

	rc = call_usermodehelper( "/usr/bin/wall", argv, envp, 0 );
	if( rc ){
		LOG( "failed to exec: %s\n ", argv[ 0 ] );
		return rc;
	}

	LOG( "execute : %s %s\n", argv[ 0 ], argv[ 1 ] );
	msleep( 100 );

	return -1;
}

module_init( exec_init );

MODULE_LICENSE( "GPL" );
MODULE_VERSION( "1.1" );