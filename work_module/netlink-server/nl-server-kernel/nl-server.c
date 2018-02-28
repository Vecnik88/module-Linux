#include <net/sock.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/proc_ns.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>

#include "nl-server.h"

#define NL_KSERV_LOG(fmt, ...)  	   \
	printk(KERN_INFO MODULE_NAME ": "  \
	       "[%s] " fmt,                \
	       __func__, ##__VA_ARGS__)

#define NL_KSERV_ERR(fmt, ...)  	   \
	printk(KERN_ERR MODULE_NAME ": "   \
	       "[%s] " fmt,                \
	       __func__, ##__VA_ARGS__)

#define NL_KSERV_DBG(fmt, ...)  	       \
	if (unlikely(nl_kserv_debug))          \
	    printk(KERN_ALERT MODULE_NAME ": " \
	           "[%s] " fmt,                \
	           __func__, ##__VA_ARGS__)

extern struct net init_net;
static struct sock *nl_socket = NULL;
static unsigned long nl_kserv_error = 0;
static unsigned long nl_kserv_norm_msg = 0;

int nl_kserv_debug = 0;
module_param_named(debug, nl_kserv_debug, int, 0644);
MODULE_PARM_DESC(debug, "Debuging this module: set in /you_path/generate_modules.sh"
                        "(0=off debug, 1=on debug)");

static struct net *get_ptr_by_vrf(char *vrf_name)
{
	struct net *net = NULL;
	struct file *file = NULL;
	struct proc_ns *ei = NULL;
	char net_path[NET_PATH];
	memset(net_path, 0, sizeof(net_path));

	snprintf(net_path, sizeof(net_path), "%s/%s", NETNS_RUN_DIR, vrf_name);
	NL_KSERV_DBG("path vrf=%s!\n", net_path);

	file = filp_open(net_path, O_RDONLY, 0);
	if (file) {
		ei = get_proc_ns(file_inode(file));
		if (ei) {
			net = get_net(ei->ns);
			if (net == NULL)
				NL_KSERV_ERR("failed net pointer is NULL!\n");

			filp_close(file, NULL);
			return net;
		}
		filp_close(file, NULL);
	}
	NL_KSERV_ERR("can't read file %s!\n", net_path);

	return net;
}

static int set_carrier_netdev(char *dev_name, char *vrf_name, uint16_t is_up)
{
	struct net *net = NULL;
	struct net_device *net_dev = NULL;

	if (vrf_name[0] != '\0') {
		net = get_ptr_by_vrf(vrf_name);
		if (net == NULL) {
			NL_KSERV_ERR("failed vrf name!\n");
			return NETLINK_SERV_ERR;
		}
	} else {
		net = &init_net;
	}
	net_dev = __dev_get_by_name(net, dev_name);
	if (net_dev) {
		if (is_up) {
			NL_KSERV_DBG("net device name=%s in vrf=%s IS UP!\n",
			             dev_name,
			             vrf_name[0] ? vrf_name : "global");
			netif_carrier_on(net_dev);
		} else {
			NL_KSERV_DBG("net device name=%s in vrf=%s IS DOWN!\n",
			             dev_name,
			             vrf_name[0] ? vrf_name : "global");
			netif_carrier_off(net_dev);
		}
		return 0;
	} else {
		for_each_net (net) {
			net_dev = __dev_get_by_name(net, dev_name);
			if (net_dev) {
				++nl_kserv_error;
				if (is_up) {
					NL_KSERV_DBG("net device name=%s is not find vrf=%s IS UP!\n", 
					             dev_name,
					             vrf_name[0] ? vrf_name : "global");
					netif_carrier_on(net_dev);
				} else {
					NL_KSERV_DBG("net device name=%s is not find vrf=%s IS DOWN!\n",
					             dev_name,
					             vrf_name[0] ? vrf_name : "global");
					netif_carrier_off(net_dev);
				}
				return 0;
			}
		}
	}
	NL_KSERV_ERR("failed vlan is not vrf!\n");

	return NETLINK_SERV_ERR;
}

static int set_carrier_vlan(struct nlmsghdr *nl_hdr)
{
	ssize_t size = 0;
	char vlan_name[IFNAMSIZ];
	struct vlan_carrier *vlan_opt = NULL;
	memset(vlan_name, 0, IFNAMSIZ);

	size = sizeof(struct request_to_kernel);
	vlan_opt = (struct vlan_carrier *)(nlmsg_data(nl_hdr) + size);
	if (vlan_opt == NULL) {
		NL_KSERV_ERR("pointer vlan_opt is null!\n");
		return NETLINK_SERV_ERR;
	} else if (vlan_opt->vid < VLAN_FIRST || vlan_opt->vid > VLAN_LAST) {
		NL_KSERV_ERR("failed number vlan!\n");
		return NETLINK_SERV_ERR;
	}
	snprintf(vlan_name, IFNAMSIZ, "%s%d", "vlan.", vlan_opt->vid);
	NL_KSERV_DBG("vlan name=%s, is_up=%d, vrf name=%s!\n",
	             vlan_name, vlan_opt->is_up,
	             vlan_opt->vrf_name[0] ? vlan_opt->vrf_name : "global");

	return set_carrier_netdev(vlan_name,
		                      vlan_opt->vrf_name,
		                      vlan_opt->is_up);
}

int set_carrier_port_channel(struct nlmsghdr *nl_hdr)
{
	int err = 0;
	ssize_t size = 0;
	char port_channel_name[IFNAMSIZ];
	struct port_channel_carrier *port_channel_opt = NULL;
	memset(port_channel_name, 0, IFNAMSIZ);

	size = sizeof(struct request_to_kernel);
	port_channel_opt = (struct port_channel_carrier *)(nlmsg_data(nl_hdr) + size);
	if (port_channel_opt == NULL) {
		NL_KSERV_ERR("pointer port_channel_opt is null!\n");
		return NETLINK_SERV_ERR;
	} else if (port_channel_opt->num < PORT_CHANNEL_FIRST ||
	           port_channel_opt->num > PORT_CHANNEL_LAST) {
		NL_KSERV_ERR("failed number port channel!\n");
		return NETLINK_SERV_ERR;
	}
	snprintf(port_channel_name, IFNAMSIZ, "%s%d", "vlan_po", port_channel_opt->num);
	NL_KSERV_DBG("sub port channel name=%s, is_up=%d, vrf name=%s!\n",
	             port_channel_name,
	             port_channel_opt->is_up,
	             port_channel_opt->vrf_name[0] ? port_channel_opt->vrf_name : "global");
	if (err == NETLINK_SERV_ERR) {
		NL_KSERV_ERR("is not set carrier port channel\n");
		return err;
	}
	err = set_carrier_netdev(port_channel_name,
	                         port_channel_opt->vrf_name,
	                         port_channel_opt->is_up);
	if (err == NETLINK_SERV_ERR) {
		NL_KSERV_ERR("is not set carrier sub port channel %s\n", port_channel_name);
		return err;
	}
	snprintf(port_channel_name, IFNAMSIZ, "%s%d%s", "po", port_channel_opt->num, "_vlan");
	NL_KSERV_DBG("sub port channel name=%s, is_up=%d, vrf name=%s!\n",
	             port_channel_name,
	             port_channel_opt->is_up,
	             port_channel_opt->vrf_name[0] ? port_channel_opt->vrf_name : "global");
	err = set_carrier_netdev(port_channel_name,
	                         port_channel_opt->vrf_name,
	                         port_channel_opt->is_up);
	if (err == NETLINK_SERV_ERR) {
		NL_KSERV_ERR("is not set carrier sub port channel %s\n", port_channel_name);
		return err;
	}
	snprintf(port_channel_name, IFNAMSIZ, "%s%d", "po", port_channel_opt->num);
	NL_KSERV_DBG("port channel name=%s, is_up=%d, vrf name=%s!\n",
	             port_channel_name,
	             port_channel_opt->is_up,
	             port_channel_opt->vrf_name[0] ? port_channel_opt->vrf_name : "global");
	err = set_carrier_netdev(port_channel_name,
	                         port_channel_opt->vrf_name,
	                         port_channel_opt->is_up);

	return err;
}

static void nl_kserv_recv_msg(struct sk_buff *skb)
{
	ssize_t size = 0;
	int pid = 0, err = 0;
	struct nlmsghdr *nl_hdr = NULL;
	struct reply_with_kernel reply;
	struct sk_buff *skb_to_user = NULL;
	struct request_to_kernel *request = NULL;
	memset(&reply, 0, sizeof(reply));

	nl_hdr = (struct nlmsghdr *)skb->data;
	if (nl_hdr == NULL) {
		NL_KSERV_ERR("failed packet!\n");
		err = NETLINK_SERV_ERR;
		goto out;
	}
	pid = nl_hdr->nlmsg_pid;
	request = (struct request_to_kernel *)nlmsg_data(nl_hdr);
	if (request == NULL) {
		NL_KSERV_ERR("failed request with kernel!\n");
		err = NETLINK_SERV_ERR;
		goto out;
	}
	switch (request->cmd) {
		case SET_VLAN_CARRIER:
			err = set_carrier_vlan(nl_hdr);
			break;
		case SET_PORT_CHANNEL_CARRIER:
			err = set_carrier_port_channel(nl_hdr);
			break;
		case GET_VLAN_CARRIER:
		case GET_PORT_CHANNEL_CARRIER:
		default:
			NL_KSERV_ERR("incorrect command to kernel!\n");
			err = NETLINK_SERV_ERR;
			break;
	}

out:
	if (err == NETLINK_SERV_ERR)
		++nl_kserv_error;
	else
		++nl_kserv_norm_msg;

	skb_to_user = nlmsg_new(MAX_PAYLOAD, GFP_KERNEL);
	if (skb_to_user == NULL) {
		++nl_kserv_error;
		NL_KSERV_ERR("failed to allocate memory!\n");
		return;
	}
	reply.err = err;
	nl_hdr = nlmsg_put(skb_to_user, 0, 0, NLMSG_DONE, MAX_PAYLOAD, 0);
	NETLINK_CB(skb_to_user).dst_group = 0;
	size = sizeof(struct reply_with_kernel);
	memcpy((void *)nlmsg_data(nl_hdr), (void *)&reply, size);
	if (nlmsg_unicast(nl_socket, skb_to_user, pid) < 0) {
		++nl_kserv_error;
		NL_KSERV_ERR("failed send message to user-space!\n");
	}
}

struct netlink_kernel_cfg cfg = {
	.input = nl_kserv_recv_msg,
};

static ssize_t nl_kserv_read_info(struct file *file, char *buf,
                                    size_t count, loff_t *ppos)
{
	static char buf_msg[LEN_MSG];
	memset(buf_msg, 0, LEN_MSG);

	snprintf(buf_msg, LEN_MSG, "%s%lu\n%s%lu\n", "Normal message: ", nl_kserv_norm_msg,
	                                             "Errors: ", nl_kserv_error);

	if (*ppos >= strlen(buf_msg)) {
		*ppos = 0;
		return 0;
	}
	if (count > strlen(buf_msg) - *ppos)
		count = strlen(buf_msg) - *ppos;
	if (copy_to_user((void *)buf, buf_msg + *ppos, count)) {
		NL_KSERV_ERR("can't copy data to user space\n");
		return -EFAULT;
	}
	*ppos += count;

	return count;
}

static ssize_t nl_kserv_control(struct file *file, const char *buf,
                                size_t count, loff_t *ppos)
{
	return 0;
}

static const struct file_operations nl_fops = {
	.owner = THIS_MODULE,
	.read = nl_kserv_read_info,
	.write = nl_kserv_control
};

static int __init nl_kserv_init(void)
{
	struct proc_dir_entry *nl_node = NULL;

	nl_socket = netlink_kernel_create(&init_net, NETLINK_SERV_PROT, &cfg);
	if (nl_socket == NULL) {
		NL_KSERV_ERR("can't create netlink socket!\n");
		return -ENOMEM;
	}
	nl_node = proc_create(MODULE_NAME, S_IFREG | S_IRUGO | S_IWUGO, NULL, &nl_fops);
	if (nl_node == NULL) {
		NL_KSERV_ERR("can't create /proc/%s!\n", MODULE_NAME);
		return -ENOENT;
	}
	NL_KSERV_LOG("loaded!\n");

	return 0;
}

static void __exit nl_kserv_exit(void)
{
	netlink_kernel_release(nl_socket);
	remove_proc_entry(MODULE_NAME, NULL);

	NL_KSERV_ERR("unloaded!\n");
}

module_init(nl_kserv_init);
module_exit(nl_kserv_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Netlink server");
MODULE_AUTHOR("Anton Mikaev (ESR Group)");
