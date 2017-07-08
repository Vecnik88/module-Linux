#include <linux/kernel.h>
#include <linux/module.h>
#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>

#define NETLINK_MY_PROTOCOL 17

#define ERR(...) printk( KERN_ERR __VA_ARGS__ )
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static struct sock* nlsk;

static void recv_cmd( struct sk_buff* skb ){
	LOG( "HELLO BRO" );
}

struct netlink_kernel_cfg cfg = {
	.input = recv_cmd,
};

static int __init nlexample_init( void ){
	nlsk = netlink_kernel_create( &init_net, NETLINK_MY_PROTOCOL, &cfg );
	if( nlsk == NULL ){
		ERR( "Can't create netlink socket\n" );
		return -ENOMEM;
	}

	LOG( "========== MODULE NETLINK INIT ==========" );

	return 0;
}

static void __exit nlexample_exit( void ){
	netlink_kernel_release( nlsk );

	LOG( "========== MODULE NETLINK REMOVE ==========" );
}

module_init( nlexample_init );
module_exit( nlexample_exit );