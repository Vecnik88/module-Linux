#include <linux/module.h>
#include <linux/version.h>
#include <linux/netdevice.h>

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )
#define ERR(...) printk( KERN_ERR __VA_ARGS__ )

static int num = 1;						// <---. число создаваемых интерфейсов
module_param( num, int, 0 );

static char* title;						// <---. префикс имени интерфейсов
module_param( title, charp, 0 );

static int digit = 1;						// <---. числовые суфиксы( по умолчанию )
module_param( digit, int, 0 );

#if ( LINUX_VERSION_CODE >= KERNEL_VERSION( 3,17,0 ) )
static int mode = 1;						// <---. режим нумерации интерфейсов
module_param( mode, int, 0 );
#endif

static struct net_device* adev[] = { NULL, NULL, NULL, NULL };

static int my_open( struct net_device* dev ){
	LOG( "Hit: my_open(%s)\n", dev->name );
	netif_start_queue( dev );

	return 0;
}

static int my_close( struct net_device* dev ){
	LOG( "Hit: my_close(%s)\n", dev->name );
	netif_stop_queue( dev );

	return 0;
}

static int stub_start_xmit( struct sk_buff* skb, struct net_device* dev ){
	dev_kfree_skb( skb );

	return 0;
}

static struct net_device_ops ndo = {
	.ndo_open = my_open,
	.ndo_stop = my_close,
	.ndo_start_xmit = stub_start_xmit,
};

static int ipos;

static void __init my_setup( struct net_device* dev ){
	int j;
	for( j = 0; j < ETH_ALEN; ++j ){
		dev->dev_addr[ j ] = ( char )( j + ipos );
	}

	ether_setup( dev );
	dev->netdev_ops = &ndo;
}


static int __init my_init( void ){
	char prefix[ 20 ];
	if( num > sizeof( adev ) / sizeof( adev[ 0 ] ) ){
		ERR( "! link number error" );
		return -EINVAL;
	}
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION( 3,17,0 ) )
	if( mode < 0 || mode > NET_NAME_RENAMED ){
		ERR( "! unknown name assign mode" );
		return -EINVAL;
	}
#endif
	LOG( "! loading network module for %d links", num );
	sprintf( prefix, "%s%s", ( title == NULL ? "fict" : title ), "%d" );
	for( ipos = 0; ipos < num; ++ipos ){
		if( !digit )
			sprintf( prefix, "%s%c", ( title == NULL ? "fict" : title ), 'a' + ipos );
#if ( LINUX_VERSION_CODE < KERNEL_VERSION( 3,17,0 ) )
		adev[ ipos ] = alloc_netdev( 0, prefix, my_setup );
#else
		adev [ ipos ] = alloc_netdev( 0, prefix, NET_NAME_UNKNOWN + mode, my_setup );
#endif
		if( register_netdev( adev[ ipos ] ) ){
			int j;
			ERR( "! failed to register" );
			for( j = ipos; j >= 0; --j ){
				if( j != ipos )
					unregister_netdev( adev[ ipos ] );
				free_netdev( adev[ ipos ] );
			}

			return -ELNRNG;
		}
	}
	LOG( "! succeeded in loadin %d devices!", num );

	return 0;
}

static void __exit my_exit( void ){
	int i;
	LOG( "! unloading network module" );
	for( i = 0; i < num; ++i ){
		unregister_netdev( adev[ i ] );
		free_netdev( adev[ i ] );
	}
}

module_init( my_init );
module_exit( my_exit );

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Vecnik88" );
MODULE_DESCRIPTION( "HO HO HO" );
