#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Vecnik88" );
MODULE_VERSION( "1.1" );
MODULE_DESCRIPTION( "Разбор slab аллокатора в Линукс" );

#define ERR(...) printk( KERN_ERR __VA_ARGS__ )
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static int size = 7;
module_param( size, int, 0 );

static int number = 31;
module_param( number, int, 0 );

static void* *line = NULL;
static int sco = 0;

#if LINUX_VERSION_CODE > KERNEL_VERSION( 2, 6, 31 )
static void co( void* p )
#else 
static void co( void* p, kmem_cache_t* c, unsigned long f )
#endif
{
	*( int* )p = ( int )p;
	++sco;
}

#define SLABNAME "my_cashe"
struct kmem_cache* cache = NULL;

static int __init init( void ){
	int i;
	if( size < sizeof( void* ) ){
		ERR( "invalid argument\n" );
		return -EINVAL;
	}
	line = kmalloc( sizeof( void* ) * number, GFP_KERNEL );
	if( line == NULL ){
		ERR( "kmalloc error\n" );
		goto mout;
	}

	for( i = 0; i < number; ++i )
		line[ i ] = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION( 2, 6, 32 )
	cache = kmem_cache_create( SLABNAME, size, 0, SLAB_HWCACHE_ALIGN, co, NULL );
#else
	cache = kmem_cache_create( SLABNAME, size, 0, SLAB_HWCACHE_ALIGN, co );
#endif

	if( cache == NULL ){
		ERR( "kmem_cache_create error \n" );
		goto cout;
	}

	for( i = 0; i < number; ++i )
		if( ( line[ i ] = kmem_cache_alloc( cache, GFP_KERNEL ) ) == NULL ){
			ERR( "kmem_cache_alloc error\n" );
			goto oout;
		}

	LOG( "allocate %d objects info slab: %s\n", number, SLABNAME );
	LOG( "object size %d bytes, full size %ld bytes\n", size, ( long ) size * number );
	LOG( "constructor called %d times\n", sco );

	return 0;

oout:
	for( i = 0; i < number; ++i )
		kmem_cache_free( cache, line[ i ] );

cout:
	kmem_cache_destroy( cache );

mout:
	kfree( line );
	return -ENOMEM;
}

static void __exit exit( void ){
	int i = 0;
	for( i = 0; i < number; ++i ){
		kmem_cache_free( cache, line[ i ] );
	}

	kmem_cache_destroy( cache );
	kfree( line );
}

module_init( init );
module_exit( exit );











