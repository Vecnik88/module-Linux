					/* 				
                                    Тестовый модуль для отслеживания 
						      производительности ядра с доступом через счетчики per cpu и блокироваинем 
					 */

#include <net/arp.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/moduleparam.h>



/* макросы для удобства */
#define ERR(...) printk( KERN_ERR __VA_ARGS__ )
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )
#define DEBUG(...) if( debug > 0 ) printk( KERN_INFO "DBG MSG " __VA_ARGS__ )

static char* link = "enps2n0";							/* имя родительского интерфейса */
module_param( link, charp, 0 );

static char* ifname = "interface_name_this";			/* имя виртуального интерфейса, создаваемого модулем */
module_param( ifname, charp, 0 );

static int debug = 0;									/* для отладки модулей */
module_param( debug, int, 0 );

struct net_device* child = NULL;

struct pcpu_lstats {
	u64 packets;
	u64 bytes;
	struct u64_stats_sync syncp;
};

static int test_open( struct net_device* dev ){
	LOG( "===== Virtual interface %s UP =====\n", dev->name );

	struct in_device* in_dev = dev->ip_ptr;
	struct in_ifaddr* ifa = in_dev->ifa_list;

	netif_start_queue( dev ); 							/* Разрешаем обработку пакетов */

	return 0;
}

static int test_stop( struct net_device* dev ){
	LOG( "===== Virtual interface %s DOWN =====\n", dev->name );

	netif_stop_queue( dev );

	return 0;
}

static netdev_tx_t test_start_xmit( struct sk_buff* skb, struct net_device* dev ){

	struct pcpu_lstats *lb_stats;
	int len;

	skb_orphan(skb);

	/* 
		Перед очередью этого пакета в netif_rx (),
			Убедитесь, что dst refcounted.
	 */

	skb_dst_force(skb);

	skb->protocol = eth_type_trans(skb, dev);

	/* нормально использовать per_cpu_ptr( ), потому что BHs отключены */

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
						   struct rtnl_link_stats64 *stats)
{
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

int all_packet( struct sk_buff* skb, struct net_device* dev,			/* обработчик всех пакетов */
				struct packet_type* pkt, struct net_device* odev ){

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


static int __ init test_init( void ){



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
