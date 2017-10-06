/* Разбор очередей преываний */

#include <linux/module.h>
#include <linux/slab.h>

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

MODULE_LICENSE( "GPL" );

static int works = 2;						// <---. число отложенных работ
module_param( works, int, S_IRUGO );

static struct workqueue_struct* my_wq;

typedef struct{
	struct work_struct my_work;
	int id;
	u32 j;
	cycles_t cycles;
} my_work_t;

static void my_wq_function( struct work_struct* work ){
	u32 j = jiffies;
	cycles_t cycles = get_cycles();
	my_work_t* wrk = ( my_work_t* ) work;
	LOG( "#%d : %010lld [%05d] => %010lld [%05d] = %06lu : context %d\n",
		 wrk->id, (long long unsigned)wrk->cycles, wrk->j,
		 (long long unsigned)cycles, j,
		 (long unsigned)( cycles - wrk->cycles ), in_atomic() );

	kfree( ( void* )wrk );

	return;
}

static int __init my_module( void ){
	int n, ret;
	if( ( my_wq = create_workqueue( "my_queue" ) ) )
		for( n = 0; n < works; ++n ){
			my_work_t* work = ( my_work_t* )kmalloc( sizeof( my_work_t ), GFP_KERNEL );
			if( work ){
				INIT_WORK( ( struct work_struct* )work, my_wq_function );
				work->id = n;
				work->j = jiffies;
				work->cycles = get_cycles();
				ret = queue_work( my_wq, ( struct work_struct* )work );
				if( !ret )
					return -ENOMEM;
			} else{
				return -ENOMEM;
			}
		} else {
			return -EBADRQC;
		}

		return 0;
}

static void __exit my_exit( void ){
	flush_workqueue( my_wq );
	destroy_workqueue( my_wq );
	return;
}

module_init( my_module );
module_exit( my_exit );