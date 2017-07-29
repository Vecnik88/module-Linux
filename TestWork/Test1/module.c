					/* Разбор макросов для передачи параетров ядру */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <linux/kernel.h>

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Vecnik88" );
MODULE_DESCRIPTION( "Разбор макросов для передачи параметров ядру" );

static int iparam = 0;
module_param( iparam, int, 0 );

static int k = 0;
module_param_named( nparam, k, int, 0 );

static bool bparam = true;
module_param( bparam, invbool, 0 );

static char* sparam;
module_param( sparam, charp, 0 );

#define FIXLEN 5

static char s[ FIXLEN ] = "";
module_param_string( cparam, s, sizeof(s), 0 );

static int aparam[] = { 0,0,0,0,0 };
static int arnum = sizeof( aparam )/sizeof( aparam[ 0 ] );
module_param_array( aparam, int, &arnum, S_IRUGO );

static int init_function( void ){
	int j = 0;
	char msg[ 40 ] = "";
	LOG( "========================================\n" );
	LOG( "iparam = %d\n", iparam );
	LOG( "nparam = %d\n", k );
	LOG( "bparam = %d\n", bparam );
	LOG( "sparam = %s\n", sparam );
	LOG( "cparam = %s {%d}\n", s, strlen( s ) );
	sprintf( msg, "aparam [ %d ] = ", arnum );

	for( j = 0; j < arnum; j++ ) 
		sprintf( msg + strlen( msg ), " %d ", aparam[ j ] );

	LOG( "%s\n", msg );
	LOG( "========================================\n" );

	return -1;
}

module_init( init_function );








