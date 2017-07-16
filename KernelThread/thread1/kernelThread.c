						/* Многопоточность в ядре */

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/delay.h>

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static int param = 3;
module_param( param, int, S_IRUGO );

static int thread( void* data ){
	LOG( "thread: child process [ %d ] is running\n", current->pid );
	ssleep( param );							/* pause */
	LOG( "thread: child process [ %d ] is completed\n", current->pid );

	return 0;
}

static int __init test_thread( void ){
	pid_t pid;
	LOG( "thread: main process [ %d ] is running\n", current->pid );
	pid = kernel_thread( thread, NULL, CLONE_FS );

	ssleep( 5 );
	LOG( "thread: main proccess [ %d ] is completed\n", current->pid );

	return -1;
}

module_init( test_thread );










