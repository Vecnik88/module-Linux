		/* 				
                                    Тестовый модуль для отслеживания
					производительности ядра с доступом через счетчики per cpu
		 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <net/dst.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/moduleparam.h>
#include <linux/etherdevice.h>

/* макросы для удобства */
#define ERR(...) printk( KERN_ERR __VA_ARGS__ )
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static char* virt_dev = "test%d";
module_param( virt_dev, charp, 0 );

struct net_device* netdev = NULL;

/*
		В struct net_device есть в конце объединение union в котором содержатся структуры pcpu_dstats, pcpu_lstats и другие, 
	при написание сетевого интерфейса нужно заглянуть туда и выбрать подходящую структуру. 
		Это объединение сделано специально для percpu и имеет пометку ввиде макроса __percpu 
 */
struct pcpu_dstats {
	u64			tx_pkts;
	u64			tx_bytes;
	u64			tx_drps;
	u64			rx_pkts;
	u64			rx_bytes;
	u64			rx_drps;
	struct u64_stats_sync	syncp;
};

int virt_rcv( struct sk_buff *skb, struct net_device *dev,
                   struct packet_type *pt, struct net_device *odev ) {
	int len = skb->len;
	struct pcpu_dstats* d_stats;

  	LOG( "Size packet: %d\n", len );

	d_stats = this_cpu_ptr( dev->dstats );								/* получаем указатель на переменную cpu этого процессора */

  	u64_stats_update_begin(&d_stats->syncp);							/* синхронизируем */
	d_stats->rx_bytes += len;
	d_stats->rx_pkts++;
	u64_stats_update_end(&d_stats->syncp);

	kfree_skb( skb );													/* уменьшает skb->user на -1 - это важно так как иначе пакет не удалится в конце */

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
	dev->dstats = netdev_alloc_pcpu_stats( struct pcpu_dstats );
	
	return dev->dstats == NULL ? -ENOMEM : 0;
}

static void virt_dev_uninit( struct net_device* dev ) {
	free_percpu( dev->dstats );
}

static int virt_open( struct net_device* dev ) {
	LOG( " ===== Device with module: %s up\n", THIS_MODULE->name );

	netif_start_queue( dev );									/* разрешаем прием пакетов */
	return 0;
}

static int virt_stop( struct net_device* dev ) {
	LOG( " ===== Device with module: %s down\n", THIS_MODULE->name );

	netif_stop_queue( dev );									/* запрещаем прием пакетов */
	return 0;
}

static int virt_start_xmit( struct sk_buff* skb, struct net_device* dev ) {
	struct pcpu_dstats* d_stats;
	int len;

	skb_tx_timestamp(skb);
	skb_orphan(skb);

	skb_dst_force(skb);

	skb->protocol = eth_type_trans(skb, dev);

	d_stats = this_cpu_ptr(dev->dstats);

	len = skb->len;
	if (likely(netif_rx(skb) == NET_RX_SUCCESS)) {				/* netif_rx() - ставит пакет в очередь либо дропает его если перегрузка */
		u64_stats_update_begin(&d_stats->syncp);
		d_stats->tx_bytes += len;
		d_stats->tx_pkts++;
		u64_stats_update_end(&d_stats->syncp);
	}

	return NETDEV_TX_OK;
}

static void virt_stats64( struct net_device* dev, struct rtnl_link_stats64* stats ) {
	int i = 0;

	for_each_possible_cpu(i) {
		const struct pcpu_dstats* d_stats;
		
		u64 tpkts = 0;								/* отправленные пакеты */
		u64 rpkts = 0;								/* полученные пакеты */
		u64 tdrops = 0;								/* отброшенные пакеты на отправку */
		u64 rbytes = 0;								/* отброшенные пакеты при получение( не валидные ) */
		u64 tbytes = 0;								/* отправленное количество байт */
		u64 rdrops = 0;								/* полученное количество байт */

		unsigned int start = 0;

		d_stats = per_cpu_ptr(dev->dstats, i);

		do {
			start = u64_stats_fetch_begin_irq(&d_stats->syncp);
			tbytes = d_stats->tx_bytes;
			tpkts = d_stats->tx_pkts;
			tdrops = d_stats->tx_drps;
			rbytes = d_stats->rx_bytes;
			rpkts = d_stats->rx_pkts;
			rdrops = d_stats->rx_drps;
		} while (u64_stats_fetch_retry_irq(&d_stats->syncp, start));

		stats->tx_bytes += tbytes;
		stats->tx_packets += tpkts;
		stats->tx_dropped += tdrops;
		stats->rx_bytes += rbytes;
		stats->rx_packets += rpkts;
		stats->rx_packets += rdrops;
	}
}

static struct net_device_ops virt_dev_ops = {
	.ndo_init = virt_dev_init,
	.ndo_uninit = virt_dev_uninit,
	.ndo_open = virt_open,
	.ndo_stop = virt_stop,
	.ndo_get_stats64 = ( void* ) virt_stats64,
	.ndo_start_xmit = virt_start_xmit,
};

void setup( struct net_device* dev ) {
	int i = 0;
	ether_setup( dev );
	dev->netdev_ops = &virt_dev_ops;
	for( i = 0; i < ETH_ALEN; ++i ) {
		dev->dev_addr[ i ] = ( char )i;									/* мак адрес */
		dev->broadcast[ i ] = ( char )i;								/* широковещательный адрес */
	}
}

static int __init virt_init( void ) {
	int error = 0;
#if ( LINUX_VERSION_CODE < ( KERNEL_VERSION( 3, 17, 0 ) ) )
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

	LOG( "===== Module <%s> loaded =====\n", THIS_MODULE->name );

	return 0;

err:
	free_netdev( netdev );
	return error;
}

static void __exit virt_exit( void ) {
	dev_remove_pack( &virt_proto );
	unregister_netdev( netdev );
	free_netdev( netdev );

	LOG( "===== Module <%s> unloaded =====\n", THIS_MODULE->name );
}

module_init( virt_init );
module_exit( virt_exit );

MODULE_LICENSE( "GPL v2" );
MODULE_AUTHOR( "Vecnik88 mail:Ve_cni_K@inbox.ru" );
