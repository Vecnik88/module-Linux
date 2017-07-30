/*
 *			Реализация сообщения пространства пользователя с пространством ядра
 *		Реализовано при помощи netlink-сокетов, с использованием библиотеки netlink
 */

#include <net/sock.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>

#define MAX_PAYLOAD 1024
#define NETLINK_PROTOCOL 17

#define ERR(...) printk( KERN_ERR __VA_ARGS__ )
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

struct sock* nl_sock = ( void* ) 0;

static void recvMessageFromUserSpace( struct sk_buff* skb ){
	int res = 0;								/* для ошибок */
	int pid = 0;								/* pid процесса назначения */

	struct sk_buff* skbSend = ( void* ) 0;					/* новый пакет */
	struct nlmsghdr* nl_hdr = ( void* ) 0;					/* заголовок пришедшего пакета */
	struct nlmsghdr* nl_hdr_send = ( void* ) 0;				/* заголовок нового пакета */

	char recvMessage[ MAX_PAYLOAD - NLMSG_HDRLEN ] = {0};			/* пришедшее сообщение */
	char* dataKernel = "Hello user space. This is Kernel!";			/* сообщение для пространства пользователя */

	nl_hdr = ( struct nlmsghdr* ) skb->data;				/* получаем данніе из пакета вместе с заголовком */
	pid = nl_hdr->nlmsg_pid;

	memcpy( recvMessage, nlmsg_data( nl_hdr ), strlen( nlmsg_data( nl_hdr ) ) );
	LOG( "Message from user space:\n\t%s", ( char* )recvMessage );

	skbSend = nlmsg_new( MAX_PAYLOAD, GFP_KERNEL );				/* инкапсулированный alloc_skb() */
	if( skbSend == NULL ){
		ERR( "Failed to allocate new memory" );
		return;
	}

	nl_hdr_send = nlmsg_put( skbSend, 0, 0, NLMSG_DONE, MAX_PAYLOAD, 0 );	// <---. формируем пакет
	NETLINK_CB( skbSend ).dst_group = 0;
	strncpy( nlmsg_data( nl_hdr_send ), dataKernel, strlen( dataKernel ) );

	res = nlmsg_unicast( nl_sock, skbSend, pid );				/* отправка сообщения в пространство пользователя */
	if( res < 0 )
		ERR( "Packet not send" );

	LOG( "Message send to user space" );
}

struct netlink_kernel_cfg cfg = {
	.input = recvMessageFromUserSpace,					/* наша функция callback */
};

static int __init netlinkInit( void ){

	nl_sock = netlink_kernel_create( &init_net, NETLINK_PROTOCOL, &cfg );

	if( nl_sock == NULL ){
		ERR( "Error create socket" );
		return -ECHILD;
	}

	LOG( "========== MODULE NETLINK INIT ==========\n" );
	return 0;
}

static void __exit netlinkExit( void ){
	netlink_kernel_release( nl_sock );

	LOG( "========== MODULE NETLINK REMOVED ==========\n" );
}

module_init( netlinkInit );
module_exit( netlinkExit );

MODULE_VERSION( "1.1" );
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Vecnik88" );
MODULE_DESCRIPTION( "HO HO HO" );
