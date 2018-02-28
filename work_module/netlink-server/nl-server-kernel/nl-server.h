#ifndef NL_SERVER_H
#define NL_SERVER_H

#ifndef __KERNEL__
#include <stdint.h>
#endif

#define MODULE_NAME                 "netlink-server"
#define NETNS_RUN_DIR               "/var/run/netns"

#define NETLINK_SERV_PROT           17
#define VRF_NAME_LEN                32
#define NET_PATH                    512
#define LEN_MSG                     1024
#define MAX_PAYLOAD                 8192

#define VLAN_FIRST                  1
#define VLAN_LAST                   4094

#define PORT_CHANNEL_FIRST          1
#define PORT_CHANNEL_LAST           12

#define NETLINK_SERV_ERR            100

/* commands */
#define SET_VLAN_CARRIER            (NETLINK_SERV_ERR + 1)
#define GET_VLAN_CARRIER            (NETLINK_SERV_ERR + 2)
#define SET_PORT_CHANNEL_CARRIER    (NETLINK_SERV_ERR + 3)
#define GET_PORT_CHANNEL_CARRIER    (NETLINK_SERV_ERR + 4)

struct vlan_carrier {
	uint16_t    vid;
	uint8_t     is_up;
	char        vrf_name[VRF_NAME_LEN];
};

struct port_channel_carrier {
	uint16_t    num;
	uint8_t     is_up;
	char        vrf_name[VRF_NAME_LEN];
};

struct request_to_kernel {
	uint16_t    cmd;
};

struct reply_with_kernel {
	uint16_t    err;
};

#endif	/* NL_SERVER_H */
