#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define MAX_PAYLOAD 1024				// <---. размер передаваемых данных
#define NETLINK_PROTOCOL 17				// <---. наш собственный протокол, по которому будет осуществляться связь между ядром и user_space

void ERR( const char* str );

enum bool { false = 0, true = 1 };

int main( int argc, char** argv ){
	int res = 0;
	int myNetlinkSocket = 0;			// <---. сокет, через который будет осуществляться связь

	struct sockaddr_nl src_addr, dst_addr;
	struct nlmsghdr *nlhSend = NULL;
	struct iovec myData;
	struct msghdr msg;

	myNetlinkSocket = socket( AF_NETLINK, SOCK_RAW/* SOCK_DGRAM */, NETLINK_PROTOCOL );
	if( myNetlinkSocket < 0 )
		ERR( "Error: create socket\n" );

	bzero( &msg, sizeof( msg ) );
	bzero( &src_addr, sizeof( src_addr ) );
	bzero( &dst_addr, sizeof( dst_addr ) );

	/* адрес источника */				/*	struct sockaddr_nl{									*/
	src_addr.nl_family = AF_NETLINK;	/*		sa_family_t nl_family;		<---. AF_NETLINK	*/
	src_addr.nl_pad = 0;				/*		unsigned short nl_pad;		<---. ноль			*/
	src_addr.nl_pid = getpid();			/*		pid_t nl_pid;				<---. ID порта		*/
	src_addr.nl_groups = 0;				/*		__u32 nl_groups; };			<---. маска групп	*/
	
	/* адрес назначения */
	dst_addr.nl_family = AF_NETLINK;
	dst_addr.nl_pad = 0;
	dst_addr.nl_pid = 0;
	dst_addr.nl_groups = 0;

	res = bind( myNetlinkSocket, ( struct sockaddr* )&src_addr, sizeof( src_addr ) );
	if(res < 0)
		ERR( "Error: binding socket" );
	
	nlhSend = ( struct nlmsghdr* )malloc( NLMSG_SPACE( MAX_PAYLOAD ) );
	if( nlhSend == NULL )
		ERR( "Error: malloc" );

	while( true ){
		nlhSend->nlmsg_len = NLMSG_SPACE( MAX_PAYLOAD );
		nlhSend->nlmsg_type = 0;
		nlhSend->nlmsg_flags = 0;
		nlhSend->nlmsg_seq = 0;
		nlhSend->nlmsg_pid = getpid();

		strcpy( NLMSG_DATA( nlhSend ), "Hello kernel. This is user space!\n" );

		myData.iov_base = ( void* )nlhSend;
		myData.iov_len = nlhSend->nlmsg_len;
	
		msg.msg_name = &dst_addr;
		msg.msg_namelen = sizeof( dst_addr );
		msg.msg_iov = &myData;
		msg.msg_iovlen = 1; 
		
		res = sendmsg( myNetlinkSocket, &msg, 0 );
		if( res < 0 )
			ERR( "Error: sendmsg" );

		bzero( nlhSend, NLMSG_SPACE( MAX_PAYLOAD ) );

		res = recvmsg( myNetlinkSocket, &msg, 0 );
		if( res < 0 )
			ERR( "Error: recvmsg" );

		printf("Recv message from kernel: %s\n", ( char* )NLMSG_DATA( nlhSend ) );

		sleep( 2 );
}

	shutdown( myNetlinkSocket, SHUT_RDWR );
	exit(EXIT_SUCCESS);
}

void ERR( const char* str ){
	printf( "%s\n", str );
	exit( EXIT_FAILURE );
}