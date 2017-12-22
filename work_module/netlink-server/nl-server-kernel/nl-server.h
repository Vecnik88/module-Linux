#ifndef NL_SERVER_H
#define NL_SERVER_H

#ifndef __KERNEL__
#include <stdint.h>
#endif

#define MODULE_NAME		"Netlink server"

#define NETLINK_SERV_PROT	17
#define MAX_PAYLOAD		8192

#define NETLINK_SERV_ERR	100

/* commands */
#define SET_VLAN_CARRIER	(NETLINK_SERV_ERR + 1)
#define GET_VLAN_CARRIER	(NETLINK_SERV_ERR + 2)

struct vlan_carrier {
	uint16_t	vid;
	uint8_t		is_up;
};

struct request_to_kernel {
	uint16_t	cmd;
};

struct reply_with_kernel {
	uint16_t	err;
};

#endif	/* NL_SERVER_H */
