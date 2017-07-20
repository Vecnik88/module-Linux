#include <linux/module.h>

MODULE_LICENSE( "GPL" );

static int __init mod_init( void );
static void __exit mod_exit( void );

module_init( mod_init );
module_exit( mod_exit );

inline void __init set_mod_name( char* res, char* path ){
	char *pb = strrchr( path, '/' ) + 1,
		 *pe = strrchr( path, '.' );

		strncpy( res, pb, pe - pb );
		sprintf( res + ( pe - pb ), "%s", ".ko" );
}
