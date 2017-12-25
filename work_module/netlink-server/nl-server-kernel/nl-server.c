#include <net/sock.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>

#include "nl-server.h"

static struct sock* nl_socket = NULL;

static int set_carrier_vlan(struct nlmsghdr *nl_hdr)
{
	struct net *net = NULL;
	char vlan_name[IFNAMSIZ] = {0};
	struct net_device *vlan_dev = NULL;
	struct vlan_carrier *vlan_opt = NULL;

	if (nl_hdr == NULL) {
		printk(KERN_ERR "Pointer nl_hdr is null\n");
		goto err;
	}

	vlan_opt = (struct vlan_carrier *)(nlmsg_data(nl_hdr) + sizeof(struct request_to_kernel));
	if (vlan_opt->vid < 1 || vlan_opt->vid > 4094) {
		printk(KERN_ERR "Failed number vlan\n");
		goto err;
	}

	snprintf(vlan_name, IFNAMSIZ, "%s%d", "vlan.", vlan_opt->vid);

	for_each_net(net) {
		vlan_dev = __dev_get_by_name(net, vlan_name);
		if (vlan_dev != NULL) {
			if (vlan_opt->is_up) {
				printk(KERN_ALERT "true is-up=%d\n", vlan_opt->is_up);
				netif_carrier_on(vlan_dev);
			}
			else {
				printk(KERN_ALERT "false is-up=%d\n", vlan_opt->is_up);
				netif_carrier_off(vlan_dev);
			}

			return 0;
		}
	}

err:
	printk(KERN_ALERT "vlan_name ERROR\n", vlan_name);
	return NETLINK_SERV_ERR;
}

static void nl_recv(struct sk_buff* skb)
{
	int pid = 0;
	struct nlmsghdr *nl_hdr = NULL;
	struct reply_with_kernel reply = {0};
	struct sk_buff *skb_to_user = NULL;
	struct request_to_kernel *request = NULL;

	nl_hdr = (struct nlmsghdr *)skb->data;
	if (nl_hdr == NULL) {
		printk(KERN_ERR "Failed packet\n");
		reply.err = NETLINK_SERV_ERR;
		goto out;
	}

	pid = nl_hdr->nlmsg_pid;
	request = (struct request_to_kernel *)nlmsg_data(nl_hdr);
	if (request == NULL) {
		printk(KERN_ERR "Failed request with kernel\n");
		reply.err = NETLINK_SERV_ERR;
		goto out;
	}

	switch (request->cmd) {
		case SET_VLAN_CARRIER:
			reply.err = set_carrier_vlan(nl_hdr);
			break;

		default:
			reply.err = NETLINK_SERV_ERR;
			break;
	}

out:
	skb_to_user = nlmsg_new(MAX_PAYLOAD, GFP_KERNEL);
	if (skb_to_user == NULL) {
		printk(KERN_ERR "Failed to allocate memory\n");
		return;
	}
	nl_hdr = nlmsg_put(skb_to_user, 0, 0, NLMSG_DONE, MAX_PAYLOAD, 0);
	NETLINK_CB(skb_to_user).dst_group = 0;
	memcpy((void *)nlmsg_data(nl_hdr), (void *)&reply, sizeof(struct reply_with_kernel));

	if (nlmsg_unicast(nl_socket, skb_to_user, pid) < 0)
		printk(KERN_ERR "Failed send message to user-space\n");
}

struct netlink_kernel_cfg cfg = {
	.input = nl_recv,
};

static int __init nl_kserver_init(void)
{
	nl_socket = netlink_kernel_create(&init_net, NETLINK_SERV_PROT, &cfg);
	if (nl_socket == NULL) {
		printk(KERN_ERR "Can't create netlink socket\n");
		return -ENOMEM;
	}
	printk(KERN_INFO MODULE_NAME ": loaded\n");

	return 0;
}

static void __exit nl_kserver_exit(void)
{
	netlink_kernel_release(nl_socket);
	printk(KERN_ERR MODULE_NAME ": unloaded\n");
}

module_init(nl_kserver_init);
module_exit(nl_kserver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Netlink server");
MODULE_AUTHOR("Anton Mikaev (ESR Group)");
