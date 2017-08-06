/* Работа с нетфильтрами, разбор параметров вызывающей функции */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>

#define ERR(...) printk( KERN_ERR __VA_ARGS__ )
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

int test_function( struct sk_buff* skb, struct net_device* dev,
				   struct packet_type* packet, struct net_device* odev ){

	LOG( "Name dev->%s\n", dev->name );
	LOG( "Name odev->%s\n", odev->name );
	LOG( "packet received with length: %u\n", skb->len );
	kfree_skb( skb );

	return skb->len;
}

#define YOUR_PROTOCOL_FOR_TEST 0x1

static struct packet_type pkt = {
	__constant_htons( ETH_P_ALL ),				/* здесь можно указать ваш тестовый протокол, 
												   но так как компьютер о нем ничего е знает, 
												   все это лучше делать через хук функцию :) */
	NULL,
	test_function,
	( void* )1,
	NULL
};

static int __init my_init( void ){
	dev_add_pack( &pkt );
	LOG( "===== NETFILTER MODULE LOADED =====\n" );

	return 0;
}

static void __exit my_exit( void ){
	dev_remove_pack( &pkt );
	LOG( "===== NETFILTER MODULE UNLOADED =====\n" );
}

module_init( my_init );
module_exit( my_exit );

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Vecnik88" );
MODULE_DESCRIPTION( "Test module" );