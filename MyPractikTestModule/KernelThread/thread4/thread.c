/* 
				Разбор методов синхронизации потоков в ядре
				Условные переменные и ожидание завершения
*/

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/delay.h>

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static int thread( void* data ){
	struct compltion* finished = ( struct completion* )data;
	struct task_struct* curr = current;
	LOG( "child process [ %d ] is running\n", curr->pid );
	msleep( 10000 );
	LOG( "child process [ %d ] is completed\n", curr->pid );
	complete( finished );

	return 0;
}

int test_thread( void ){
	DECLARE_COMPLETION( finished );
	struct task_struct* curr = current;
	LOG( "main process [ %d ] is running\n", curr->pid );
	pid_t pid = kernel_thread( thread, &finished, CLONE_FS );
	msleep( 5000 );
	wait_for_completion( &finished );

	LOG( "main process [ %d ] is completed\n", curr->pid );

	return -1;
}

module_init( test_thread );

MODULE_LICENSE( "GPL" );
MODULE_VERSION( "1.1" );
MODULE_DESCRIPTION( "Разбор методов синхронизации в линукс" );