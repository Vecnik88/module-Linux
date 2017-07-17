				/* Разбор различных блокировок при многопоточности в ядре */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/semaphore.h>
#include <linux/sched.h>

#include "prefix.c"

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static int num = 2;								/* количество потоков */
module_param( num, int, 0 );

static int rep = 100;							/* количество повторений */
module_param( rep, int, 0 );

static int sync = -1;							/* уровень синхронизации */
module_param( sync, int, 0 );

static int max_level = 2;						/* уровень вложенности */
module_param( max_level, int, 0 );

//static DECLARE_MUTEX( sema );
struct semaphore sema;

static long locked = 0;
static long loop_func( int lvl ){
	long n = 0;
	if( lvl == sync ){
		down( &sema );
		++locked;
	}
	if( lvl == 0 ){
		const int tick = 1;
		msleep( tick );
		n = 1;
	} else{
		int i;
		for( i = 0; i < rep; ++i ){
			n += loop_func( lvl - 1 );
		}
	}

	if( lvl == sync )
		up( &sema );

	return n;
}

struct param{
	int num;
	struct completion finished;
};

#define IDENT "mlock_thread_%d"
static int thread_func( void* data ){
	long n = 0;
	struct param* parent = ( struct param* )data;
	int num = parent->num - 1;
	struct task_struct* t1 = NULL;
	struct param parm;
	LOG( "! %s is running\n", st( num ) );

	if( num > 0 ){
		init_completion( &parm.finished );
		parm.num = num;
		t1 = kthread_run( thread_func, ( void* )&parm, IDENT, num );
	}
	n = loop_func( max_level );
	if( t1 != NULL )
		wait_for_completion( &parm.finished );

	complete( &parent->finished );
	LOG( "! %s do %ld units\n", st( num ), n );

	return 0;
}

static int test_mlock( void ){
	sema_init( &sema, 1 );
	struct task_struct* t1;
	struct param parm;
	unsigned j1 = jiffies, j2;

	if( sync > max_level )
		sync = -1;									/* no synchronize */

	LOG( "! repeat %d times in %d levels; synch. in level %d\n", rep, max_level, sync );

	init_completion( &parm.finished );
	parm.num = num;
	t1 = kthread_run( thread_func, ( void* )&parm, IDENT, num );
	wait_for_completion( &parm.finished );
	LOG( "! %s is finished\n", st( num ) );
	j2 = jiffies - j1;
	LOG( "!! working time was %d.%ld seconds, locked %ld times\n",
		  j2 / HZ, ( j2 * 10 ) / HZ % 10, locked );

	return -1;
}

module_init( test_mlock );

MODULE_LICENSE( "GPL" );
MODULE_VERSION( "1.1" );
MODULE_DESCRIPTION( "Разбор различных блокировок при многопоточности в ядре" );