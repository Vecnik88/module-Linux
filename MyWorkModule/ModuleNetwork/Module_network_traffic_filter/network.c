										/* 				Модуль ядра с фильтрацией трафика различных пакетов, 
											Скелет, в котором можно делать абсолютно все что нужно с приходящими пакетами
											Через хук-функцию удобнее, но когда нужен дополнительный сетевой интерфейс, можно поступить и так
										 */

#include <net/arp.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/moduleparam.h>

#define ERR(...) printk( KERN_ERR __VA_ARGS__ )
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )
#define DEBUG(...) if( debug > 0 ) printk( KERN_INFO "DBG MSG " __VA_ARGS__ )

static char* link = "enps2n0";							/* имя родительского интерфейса */
module_param( link, charp, 0 );

static char* ifname = "interface_name_this";			/* имя виртуального интерфейса, создаваемого модулем */
module_param( ifname, charp, 0 );

static int debug = 0;									/* для отладки модулей */
module_param( debug, int, 0 );

struct net_device* virt = NULL;

struct priv{											/* структура для различной информации нашего интерфеса, здесь можно размещать любую информацию, размещается в хвосте при создании виртуального интерфейса */
	struct net_device* parent;							/* родительский интерфейс */
	struct net_device_stats stats;						/* статистика виртуального интерфейса, можно занести кол-во отправленных пакетов, байт и т.д. */
	char name_author[30];								/* для примера еще добавил имя автора */
};

static char* strIP( u32 addr ) {     					/* диагностика IP в точечной нотации */
   static char saddr[ MAX_ADDR_LEN ];
   sprintf( saddr, "%d.%d.%d.%d",
            ( addr ) & 0xFF, ( addr >> 8 ) & 0xFF,
            ( addr >> 16 ) & 0xFF, ( addr >> 24 ) & 0xFF
          );

   return saddr;
}

static int network_open( struct net_device* dev ){		/* Вызывается при поднятии сетевого интерфейса(UP) */
	LOG( "Virtual interface %s UP.\n", dev->name );

	struct in_device *in_dev = dev->ip_ptr;
	struct in_ifaddr *ifa = in_dev->ifa_list;

	netif_start_queue( dev );

	return 0;
}

static int network_stop( struct net_device* dev ){		/* Вызывается при DOWN сетевого интерфейса */
	LOG( "Virtual interface %s DOWN.\n", dev->name );

	netif_stop_queue( dev );

	return 0;
}

static struct net_device_stats* network_get_stats( struct net_device *dev ) {		/* Статитистика виртуального сетевого интерфейса */
   return &( netdev_priv( dev )->stats );
}

static netdev_tx_t network_start_xmit( struct sk_buff* skb, struct net_device* dev ){
	++( netdev_priv( dev )->stats.tx_packets );
	( netdev_priv( dev )->stats.tx_packets ) += skb->len;
	skb->dev = ( netdev_priv( dev )->parent );
	skb->priority = 1;

	dev_queue_xmit( skb );													/* send packet */

	return 0;
}

static struct net_device_ops network_function = {
	.ndo_open = network_open,
	.ndo_stop = network_stop,
	.ndo_get_stats = network_get_stats,
	.ndo_start_xmit = network_start_xmit
};

int arp_rcv_pack( struct sk_buff* skb, struct net_device* dev,
				  struct packet_type* pkt, struct net_device* odev ){		/* обработчик пакетов arp */

	/* Здесь мы можем делать все что нам нужно с пакетами, подмену пакетов, блокирование и тд. 
		Проще это все делать через хук-функцию( намного проще, но такой способ тоже иногда необходим :) ) 
	 */

	LOG( "Packet is arp, len packet = %d\n", skb->len );

	kfree_skb( skb );
	
	return skb->len;
}

static struct packet_type arp_proto = {
	.type = __constant_htons( ETH_P_ARP ),
	.dev = NULL,
	.func = arp_rcv_pack,
	.af_packet_priv = ( void* ) 1,
	.list_head = NULL
}; 

int udp_rcv_pack( struct sk_buff* skb, struct net_device* dev,				/* обработчик пакетов udp */
				  struct packet_type* pkt, struct net_device* odev ){

	/* 
		Здесь мы можем делать все что нам нужно с пакетами, подмену пакетов, блокирование и тд. 
		Проще это все делать через хук-функцию( намного проще, но такой способ тоже иногда необходим :) ) 
	 */

	LOG( "Packet is udp, len packet = %d\n", skb->len );

	kfree_skb( skb );
	
	return skb->len;
}

static struct packet_type udp_proto = {
	.type = __constant_htons( ETH_P_UDP ),
	.dev = NULL,
	.func = udp_rcv_pack,
	.af_packet_priv = ( void* ) 1,
	.list_head = NULL
}; 

int icmp_rcv_pack( struct sk_buff* skb, struct net_device* dev,				/* обработчик пакетов icmp */
				  struct packet_type* pkt, struct net_device* odev ){

	/*	
		Здесь мы можем делать все что нам нужно с пакетами, подмену пакетов, блокирование и тд. 
		Проще это все делать через хук-функцию( намного проще, но такой способ тоже иногда необходим :) ) 
	 */

	LOG( "Packet is ICMP, len packet = %d\n", skb->len );

	kfree_skb( skb );
	
	return skb->len;
}

static struct packet_type icmp_proto = {
	.type = __constant_htons( ETH_P_ICMP ),
	.dev = NULL,
	.func = icmp_rcv_pack,
	.af_packet_priv = ( void* ) 1,
	.list_head = NULL
}; 

int ip_v4_rcv_pack( struct sk_buff* skb, struct net_device* dev,			/* обработчик пакетов ip_v4 */
				  struct packet_type* pkt, struct net_device* odev ){

	/* 
		Здесь мы можем делать все что нам нужно с пакетами, подмену пакетов, блокирование и тд. 
		Проще это все делать через хук-функцию( намного проще, но такой способ тоже иногда необходим :) ) 
	 */
	LOG( "Packet is ip_v4, len packet = %d\n", skb->len );

	kfree_skb( skb );
	
	return skb->len;
}

static struct packet_type ip_v4_proto = {
	.type = __constant_htons( ETH_P_IP ),
	.dev = NULL,
	.func = ip_v4_rcv_pack,
	.af_packet_priv = ( void* ) 1,
	.list_head = NULL
}; 

void setup( struct net_device *dev ){
      int i = 0;

      ether_setup( dev );

      memset( netdev_priv( dev ), 0, sizeof( struct priv ) );

      dev->netdev_ops = &network_function;
      for( i = 0; i < ETH_ALEN; ++i )
         dev->dev_addr[ i ] = (char)(i + 10 );

}

static int __init network_init( void ){
	int errno = 0;
	struct priv* priv;
	char ifstr[ 40 ] = "";
	sprintf( ifstr, "%s%s", ifname, "%d" );

/* создаем виртуальный интерфейс, после версии ядра 3.17.0 API изменился */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0))
	child = alloc_netdev( sizeof( struct priv ), ifstr, setup );
#else
	child = alloc_netdev( sizeof( struct priv ), ifstr, NET_NAME_UNKNOWN, setup );
#endif

	if( child == NULL ){
		ERR( "Error: alloc_netdev: %s" THIS_MODULE->name );
		return -ENOMEM;
	}

	priv = netdev_priv( child );								/* возвращает указатель на структуру priv, где мы можем хранить свои любые произвольные данные */
	priv->parent = dev_get_by_name( &init_net, link );			/* определяем link родительским устройство( ф-ция ищет по имени девайс и возвращает указатель на него ) */
	if( !priv->parent ) {
    	ERR( "%s: no such net: %s", THIS_MODULE->name, link );
     	errno = -ENODEV; 
    	goto err;
   }

   if( priv->parent->type != ARPHRD_ETHER && priv->parent->type != ARPHRD_LOOPBACK ) {
    	ERR( "%s: illegal net type", THIS_MODULE->name );
    	errno = -EINVAL; 
    	goto err;
   }

	memcpy( child->dev_addr, priv->parent->dev_addr, ETH_ALEN );
	memcpy( child->broadcast, priv->parent->broadcast, ETH_ALEN );

	if( ( errno = dev_alloc_name( child, child->name ) ) ) {	 
	/* dev_alloc_name присваивает устройству имя, 
		просматривая свою таблицу и вставляя его в подходящее место.
		Формат строки "string%d" - ф-ция сама назначит подходящий номер 
	 */
		ERR( "%s: allocate name, error %i", THIS_MODULE->name, errno );
		err = -EIO; 
		goto errno;
	}
	register_netdev( child );
	arp_proto.dev = udp_proto.dev = icmp_proto.dev = ip4_proto.dev = priv->parent; 				// <---. отслеживать будет трафик только с родительского интерфейса

	/* добавляем наши обработчики фреймов */
	dev_add_pack( &arp_proto );									
	dev_add_pack( &ip4_proto );
	dev_add_pack( &udp_proto );
	dev_add_pack( &icmp_proto );

	LOG( "===== MODULE NETWORK LOADED =====\n" );

	return 0;

err:
	free_netdev( child );
	return errno;
}

static void __exit network_exit( void ){
	struct priv *priv= netdev_priv( child );

	dev_remove_pack( &arp_proto );    							// удалить обработчик фреймов
	dev_remove_pack( &ip4_proto );    							// удалить обработчик фреймов
	dev_remove_pack( &udp_proto );    							// удалить обработчик фреймов
	dev_remove_pack( &icmp_proto );    							// удалить обработчик фреймов

	unregister_netdev( child );
	dev_put( priv->parent );									/* удаляет созданную ссылку на устройство, чтобы оно было нормально освобождено " this_cpu_dec(*dev->pcpu_refcnt); "*/
	free_netdev( child );

	LOG( "===== MODULE NETWORK UNLOADED =====\n" );
}

module_init( network_init );
module_exit( network_exit );

MODULE_LICENSE( "GPL" );
MODULE_VERSION( "1.2" );
MODULE_AUTHOR( "Vecnik88" );
MODULE_DESCRIPTION( "Filter traffic for the virtual interface" );