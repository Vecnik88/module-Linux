#include "common.c"

static char* parm1 = "";
module_param( parm1, charp, 0 );

static char* parm2 = "";
module_param( parm2, charp, 0 );

static char this_mod_file[ 40 ];

static int __init mod_init( void ){
	set_mod_name( this_mod_file, __FILE__ );
	printk( "+ модуль %s загружен: parm1 = %s, parm2 = %s\n", this_mod_file, parm1, parm2 );

	return 0;
}

static void __exit mod_exit( void ){
	printk( "+ мoдуль %s выгружен\n", this_mod_file );
}