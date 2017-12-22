#include "if-mgr_control_vlan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

static int IF_MGR_NL_SERVER_FD = -1;

int if_mgr_create_connect_to_kernel_space()
{
	struct sockaddr_nl src_addr; memset(&src_addr, 0, sizeof(struct sockaddr_nl));

	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();

	IF_MGR_NL_SERVER_FD = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_SERV_PROT);
	if (IF_MGR_NL_SERVER_FD < 0)
	{
		SYSLOG_IF_MGR_ERR("<%s> error create socket", __func__);
		return -1;
	}
	if (bind(IF_MGR_NL_SERVER_FD, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0)
	{
		SYSLOG_IF_MGR_ERR("<%s> error bind netlink socket", __func__);
		close(IF_MGR_NL_SERVER_FD);
		return -1;
	}

	struct timeval tv = {.tv_sec = 0, .tv_usec = 100000};
	if (-1 == setsockopt(IF_MGR_NL_SERVER_FD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)))
	{
		SYSLOG_IF_MGR_ERR("<%s> failed to setsockopt (SO_RCVTIMEO)", __func__);
		close(IF_MGR_NL_SERVER_FD);
		return -1;
	}
	if (-1 == setsockopt(IF_MGR_NL_SERVER_FD, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)))
	{
		SYSLOG_IF_MGR_ERR("<%s> failed to setsockopt (SO_SNDTIMEO)", __func__);
		close(IF_MGR_NL_SERVER_FD);
		return -1;
	}

	return 0;
}

int if_mgr_vlan_carrier_change(ifmgr_t * ifmgr, struct vlan_carrier vlan_opt)
{
	if (PTR_IS_NULL(ifmgr)) return -1;
	if (vlan_opt.vid == 1) return 0;

	struct nlmsghdr* nlh = NULL;
	struct iovec iov; memset(&iov, 0, (sizeof(struct iovec)));
	struct msghdr msg; memset(&msg, 0, sizeof(struct msghdr));
	struct sockaddr_nl dst_addr; memset(&dst_addr, 0, sizeof(struct sockaddr_nl));
	struct request_to_kernel request;
	struct reply_with_kernel *reply = NULL;

	dst_addr.nl_family = AF_NETLINK;

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_type = 0;
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_pid = getpid();

	request.cmd = SET_VLAN_CARRIER;

	memcpy((void *)NLMSG_DATA(nlh),(void *)&request, sizeof(struct request_to_kernel));
	memcpy((void *)NLMSG_DATA(nlh) + sizeof(struct request_to_kernel),(void *)&vlan_opt, sizeof(struct vlan_carrier));

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	msg.msg_name = (void *)&dst_addr;
	msg.msg_namelen = sizeof(dst_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	if (sendmsg(IF_MGR_NL_SERVER_FD, &msg, 0) <= 0)
	{
		SYSLOG_IF_MGR_ERR("<%s> error send message to kernel", __func__);
		free(nlh);
		return -1;
	}

	if (recvmsg(IF_MGR_NL_SERVER_FD, &msg, 0) <= 0)
	{
		SYSLOG_IF_MGR_ERR("<%s> error read message with kernel", __func__);
		free(nlh);
		return -1;
	}

	reply = (struct reply_with_kernel *)NLMSG_DATA(nlh);
	if (NETLINK_SERV_ERR == reply->err)
	{
		SYSLOG_IF_MGR_ERR("<%s> netlink server is not set vlan", __func__);
		free(nlh);
		return -1;
	}

	free(nlh);
	return 0;
}
