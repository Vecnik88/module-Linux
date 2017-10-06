#include <linux/module.h>
#include <linux/interrupt.h>

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

MODULE_LICENSE( "GPL" );

static cycles_t cycles1, cycles2;
static u32 j1, j2;
static int context;

char tasklet_data[] = "Tasclet function work";

void tasklet_function( unsigned long data ){
	context = in_atomic();
	j2 = jiffies;
	cycles2 = get_cycles();
	LOG( "%010lld [ %05d ] : %s in contxt %d\n",
		 ( long long unsigned ) cycles2, j2, ( char* )data, context );

	return;
}

DECLARE_TASKLET( my_tasklet, tasklet_function, ( unsigned long )&tasklet_data );

int init_module( void ){
	context = in_atomic();
	j1 = jiffies;
	cycles1 = get_cycles();
	tasklet_schedule( &my_tasklet );

	LOG( "%010lld [ %05d ] : tasklet was scheduled in contxt %d\n",
		 ( long long unsigned ) cycles1, j1, context );

	LOG( "========== MODULE TASKLET LOAD ==========" );

	return 0;
}

void cleanup_module( void ){
	tasklet_kill ( &my_tasklet );

	LOG( "========== MODULE TASKLET UNLOAD ==========" );
	return;
}