/*
 *					Разбор службы времени в ядре.
 *		Измеряем интервал времени, закоторое модуль был загружен в ядро.
 *
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/types.h>

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static u32 j = 0;

static int __init init( void ){
	j = jiffies;
	LOG( "module: jiffies on start = %X\n", j );

	return 0;
}

static void __exit exit( void ){
	static u32 j1 = 0;
	j1 = jiffies;
	LOG( "module: jiffies on finish = %X\n", j1 );
	j = j1 - j;
	LOG( "module: interval of life = %d\n", j / HZ );

	return;
}

module_init( init );
module_exit( exit );
