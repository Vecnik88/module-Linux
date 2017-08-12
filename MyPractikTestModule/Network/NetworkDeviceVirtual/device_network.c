								/* 		
									Создание виртуального девайса, который ничего не делает версия 4.4
								 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/moduleparam.h>

/* макросы для удобства */
#define ERR(...) printk( KERN_ERR __VA_ARGS__ )
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static char* virt_dev = "test%d";
module_param( virt_dev, charp, 0 );

struct net_device* netdev = NULL;

int virt_rcv( struct sk_buff *skb, struct net_device *dev,
                   struct packet_type *pt, struct net_device *odev ) {
   LOG( "Size packet: %d\n", skb->len );
   kfree_skb( skb );

   return skb->len;
};

static struct packet_type virt_proto = {
   __constant_htons( ETH_P_ALL ),
   NULL,
   virt_rcv,
   ( void* )1,
   NULL
};
static int virt_open( struct net_device* dev ) {
	LOG( " ===== Device with module: %s up\n", THIS_MODULE->name );

	netif_start_queue( dev );
	return 0;
}

static int virt_stop( struct net_device* dev ) {
	LOG( " ===== Device with module: %s down\n", THIS_MODULE->name );

	netif_stop_queue( dev );
	return 0;
}

static int virt_start_xmit( struct sk_buff* skb, struct net_device* dev ) {

	return 0;
}

static struct net_device_ops virt_dev_ops = {
	.ndo_open = virt_open,
	.ndo_stop = virt_stop,
	.ndo_start_xmit = virt_start_xmit,
};

void setup( struct net_device* dev ) {
	int i = 0;
	ether_setup( dev );
	dev->netdev_ops = &virt_dev_ops;
	for( i = 0; i < ETH_ALEN; ++i ) {
		dev->dev_addr[ i ] = ( char )i;
		dev->broadcast[ i ] = ( char )i;
	}
}

static int __init virt_init( void ) {
	int error = 0;
	netdev = alloc_netdev( 0, virt_dev, NET_NAME_UNKNOWN, setup );

	if( netdev == NULL ) {
		ERR( "%s: error alloc_netdev\n", THIS_MODULE->name );
		return -ENOMEM;
	}

	error = dev_alloc_name( netdev, netdev->name );
	if( error != 0 ) {
		ERR( "%s: error dev_alloc_name, error = %d\n", THIS_MODULE->name, error );
		error = -EIO;
		goto err;
	}
	register_netdev( netdev );

	virt_proto.dev = netdev;
	dev_add_pack( &virt_proto );

	LOG( "===== Module %s loaded =====\n", THIS_MODULE->name );

	return 0;

err:
	free_netdev( netdev );
	return error;
}

static void __exit virt_exit( void ) {
	dev_remove_pack( &virt_proto );
	unregister_netdev( netdev );
	free_netdev( netdev );

	LOG( "===== Module %s unloaded =====\n", THIS_MODULE->name );
}

module_init( virt_init );
module_exit( virt_exit );

MODULE_LICENSE( "GPLv2" );
MODULE_AUTHOR( "Vecnik88 mail:Ve_cni_K@inbox.ru" );