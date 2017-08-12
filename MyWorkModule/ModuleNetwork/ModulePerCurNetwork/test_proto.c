		/* 				
                                    Тестовый модуль для отслеживания
			производительности ядра с доступом через счетчики per cpu и блокированием 
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

struct pcpu_dstats {				/* структура для отслеживания статистики */
	u64			tx_packets;			/* будем отслеживать пакеты и байты */
	u64			tx_bytes;
	struct u64_stats_sync	syncp;
};

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

static int virt_dev_init( struct net_device* dev ) {
	dev->lstats = netdev_alloc_pcpu_stats( struct pcpu_lstats );
	
	return dev->lstats == NULL ? -ENOMEM : 0;
}

static int virt_dev_uninit( struct net_device* dev ) {
	free_percpu( dev->lstats );
}

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

static void virt_stats64( struct net_device* dev, struct rtnl_link_stats64* stats ) {
	int i = 0;
	u64 bytes = 0, packets = 0;

	for_each_possible_cpu(i) {
		const struct pcpu_lstats *nl_stats;
		u64 tbytes, tpackets;
		unsigned int start;

		nl_stats = per_cpu_ptr(dev->lstats, i);

		do {
			start = u64_stats_fetch_begin_irq(&nl_stats->syncp);
			tbytes = nl_stats->bytes;
			tpackets = nl_stats->packets;
		} while (u64_stats_fetch_retry_irq(&nl_stats->syncp, start));

		packets += tpackets;
		bytes += tbytes;
	}

	stats->rx_packets = packets;
	stats->tx_packets = 0;

	stats->rx_bytes = bytes;
	stats->tx_bytes = 0;
}

static struct net_device_ops virt_dev_ops = {
	.ndo_init = virt_dev_init,
	.ndo_uninit = virt_dev_uninit,
	.ndo_open = virt_open,
	.ndo_stop = virt_stop,
	.ndo_start_xmit = virt_start_xmit,
	.ndo_get_stats64 = virt_stats64,
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0))
	netdev = alloc_netdev( 0, virt_dev, setup );
#else
	netdev = alloc_netdev( 0, virt_dev, NET_NAME_UNKNOWN, setup );
#endif
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

/*#include <net/arp.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/moduleparam.h>


#define ERR(...) printk( KERN_ERR __VA_ARGS__ )
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )
#define DEBUG(...) if( debug > 0 ) printk( KERN_INFO "DBG MSG " __VA_ARGS__ )

static char* link = "enps2n0";							8
module_param( link, charp, 0 );

static char* ifname = "interface_name_this";			8
module_param( ifname, charp, 0 );

static int debug = 0;									8
module_param( debug, int, 0 );

struct net_device* child = NULL;

struct pcpu_lstats {
	u64 packets;
	u64 bytes;
	struct u64_stats_sync syncp;
};

static int test_open( struct net_device* dev ) {
	LOG( "===== Virtual interface %s UP =====\n", dev->name );

	struct in_device* in_dev = dev->ip_ptr;
	struct in_ifaddr* ifa = in_dev->ifa_list;

	netif_start_queue( dev ); 							

	return 0;
}

static int test_stop( struct net_device* dev ) {
	LOG( "===== Virtual interface %s DOWN =====\n", dev->name );

	netif_stop_queue( dev );

	return 0;
}

static netdev_tx_t test_start_xmit( struct sk_buff* skb, struct net_device* dev ) {

	struct pcpu_lstats *lb_stats;
	int len;

	skb_orphan(skb);

	/* 
		Перед очередью этого пакета в netif_rx (),
			Убедитесь, что dst refcounted.
	

	skb_dst_force(skb);

	skb->protocol = eth_type_trans(skb, dev);

	/* нормально использовать per_cpu_ptr( ), потому что BHs отключены 

	lb_stats = this_cpu_ptr(dev->lstats);

	len = skb->len;
	if (likely(netif_rx(skb) == NET_RX_SUCCESS)) {
		u64_stats_update_begin(&lb_stats->syncp);
		lb_stats->bytes += len;
		lb_stats->packets++;
		u64_stats_update_end(&lb_stats->syncp);
	}

	return NETDEV_TX_OK;
}

static struct rtnl_link_stats64* dummy_get_stats64(struct net_device *dev,
						   struct rtnl_link_stats64 *stats) {
	int i;

	for_each_possible_cpu(i) {
		const struct pcpu_dstats *dstats;
		u64 tbytes, tpackets;
		unsigned int start;

		dstats = per_cpu_ptr(dev->dstats, i);
		do {
			start = u64_stats_fetch_begin_bh(&dstats->syncp);
			tbytes = dstats->tx_bytes;
			tpackets = dstats->tx_packets;
		} while (u64_stats_fetch_retry_bh(&dstats->syncp, start));
		stats->tx_bytes += tbytes;
		stats->tx_packets += tpackets;
	}

	return stats;
}

static struct net_device_ops network_function = {
	.ndo_open = test_open,
	.ndo_stop = test_stop,
	.ndo_get_stats64 = loopback_get_stats64,
	.ndo_start_xmit = test_start_xmit
};

int all_packet( struct sk_buff* skb, struct net_device* dev,			/* обработчик всех пакетов 
				struct packet_type* pkt, struct net_device* odev ) {

	LOG( "Function all_packet recv_packet= %d\n", skb->len );

	kfree_skb( skb );
	
	return skb->len;
}

static struct packet_type ip_v4_proto = {
	.type = __constant_htons( ETH_P_ALL ),
	.dev = NULL,
	.func = ip_v4_rcv_pack,
	.id_match = ( void* ) 1,
	.af_packet_priv = NULL,
};

void setup( struct net_device* dev ) {
	int i;
	ether_setup( dev );
	de->netdev_ops = &network_function;
	for( i = 0; i < ETH_ALEN; ++i )
		dev->dev_addr[ i ] = ( char ) i;

}

static int __ init test_init( void ) {
	int error = 0;
	child = alloc_netdev( 0, "test_module", NET_NAME_UNKNOWN, setup );
	if( child == NULL ) {
		ERR( "%s: allocate error", THIS_MODULE->name );
		return -ENOMEM;
	}


	LOG( "===== MODULE TEST PROTO INIT =====\n" );

	return 0;
}

static void __exit test_exit( void ){



	LOG( "===== MODULE TEST PROTO REMOVED =====\n" );
}

module_init( test_init );
module_exit( test_exit );

MODULE_VERSION( "1.1" );
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Vecnik88" );
MODULE_DESCRIPTION( " Тестовый модуль для изучения переменных процессора (per cpu),"
					" чтобы в дальнейшем их модифицировать для улучшения производительности процессора" );
*/