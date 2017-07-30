/* Отправка сообщений в kernel_space из user_space */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define SIZE_DATA 1024
#define NETLINK_MY_PROTOCOL 17

enum {
	false = 0, 
	true = 1
};

int ERR( const char* str );

int main( int argc, char** argv ){
	int netlinkSocket = 0, res = 0;
	struct sockaddr_nl src_addr, dest_addr;
	struct msghdr msg;
	struct nlmsghdr* nlh = NULL;
	struct iovec iov;

	netlinkSocket = socket( AF_NETLINK, SOCK_DGRAM, NETLINK_MY_PROTOCOL );
	if( netlinkSocket < 0 ){
		ERR( "Error: create socket\n" );
	}

	bzero( &src_addr, sizeof( src_addr ) );
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();
	src_addr.nl_groups = 0;

	res = bind( netlinkSocket, ( struct sockaddr* )&src_addr, sizeof( src_addr ) );
	if( res < 0 )
		ERR( "Error: binding socket\n" );

	bzero( &dest_addr, sizeof( dest_addr ) );
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;							// <---. всегда 0 так как сообщение для ядра
	dest_addr.nl_groups = 0;

	nlh = ( struct nlmsghdr* )malloc( NLMSG_SPACE( SIZE_DATA ) );
	nlh->nlmsg_len = NLMSG_SPACE( SIZE_DATA );
	nlh->nlmsg_type = 0;
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_pid = getpid();

	strcpy( NLMSG_DATA( nlh ), "Hello kernel, this is user space" );
	iov.iov_base = ( void* )nlh;
	iov.iov_len = nlh->nlmsg_len;

	bzero( &msg, sizeof( msg ) );

	msg.msg_name = ( void* )&dest_addr;
	msg.msg_namelen = sizeof( dest_addr );
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	while( true ){
		res = sendmsg( netlinkSocket, &msg, 0 );
		if( res < 0 )
			ERR( "Error: sendmsg" );

		printf( "Send message to kernel\n" );
		sleep(2);
	
}
	free( nlh );
	close( netlinkSocket );

	return ( EXIT_SUCCESS );
}

int ERR( const char* str ){
	printf( "%s\n", str );
	exit( EXIT_FAILURE );
}
