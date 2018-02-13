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

#define NL_KSERV_LOG(fmt, ...)             \
	printk(KERN_INFO MODULE_NAME ": "  \
		   "[%s] " fmt,            \
		   __func__, ##__VA_ARGS__)

#define NL_KSERV_ERR(fmt, ...)             \
	printk(KERN_ERR MODULE_NAME ": "   \
		   "[%s] " fmt,            \
		   __func__, ##__VA_ARGS__)

#define NL_KSERV_DBG(fmt, ...)                     \
	if (unlikely(nl_kserver_debug))            \
		printk(KERN_ALERT MODULE_NAME ": " \
		       "[%s] " fmt,                \
		       __func__, ##__VA_ARGS__)

extern struct net init_net;
static struct sock *nl_socket = NULL;
static unsigned long nl_kserver_error = 0;
static unsigned long nl_kserver_norm_msg = 0;

int nl_kserver_debug = 0;
module_param_named(debug_is_on, nl_kserver_debug, int, 0644);
MODULE_PARM_DESC(debug_is_on, "Debuging this module: set in generate_modules.sh"
                              "(0=off debug, 1=on debug)");

static struct net *get_pointer_by_vrf(char *vrf_name)
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

static int set_carrier_vlan(struct nlmsghdr *nl_hdr)
{
	struct net *net = NULL;
	char vlan_name[IFNAMSIZ];
	struct net_device *vlan_dev = NULL;
	struct vlan_carrier *vlan_opt = NULL;
	memset(vlan_name, 0, IFNAMSIZ);

	vlan_opt = (struct vlan_carrier *)(nlmsg_data(nl_hdr) + sizeof(struct request_to_kernel));
	if (vlan_opt == NULL) {
		NL_KSERV_ERR("pointer vlan_opt is null!\n");
		return NETLINK_SERV_ERR;
	} else if (vlan_opt->vid < 1 || vlan_opt->vid > 4094) {
		NL_KSERV_ERR("failed number vlan!\n");
		return NETLINK_SERV_ERR;
	}
	if (vlan_opt->vrf_name[0]) {
		net = get_pointer_by_vrf(vlan_opt->vrf_name);
		if (net == NULL) {
			NL_KSERV_ERR("failed vrf name!\n");
			return NETLINK_SERV_ERR;
		}
	} else {
		net = &init_net;
	}
	snprintf(vlan_name, IFNAMSIZ, "%s%d", "vlan.", vlan_opt->vid);
	NL_KSERV_DBG("vlan name=%s, is_up=%d, vrf name=%s!\n",
				 vlan_name, vlan_opt->is_up,
				 vlan_opt->vrf_name[0] ? vlan_opt->vrf_name : "global");

	vlan_dev = __dev_get_by_name(net, vlan_name);
	if (vlan_dev) {
		if (vlan_opt->is_up) {
			NL_KSERV_DBG("vlan name=%s in vrf=%s IS UP!\n",
						 vlan_name,
						 vlan_opt->vrf_name[0] ? vlan_opt->vrf_name : "global");
			netif_carrier_on(vlan_dev);
		} else {
			NL_KSERV_DBG("vlan name=%s in vrf=%s IS DOWN!\n",
						 vlan_name,
						 vlan_opt->vrf_name[0] ? vlan_opt->vrf_name : "global");
			netif_carrier_off(vlan_dev);
		}
		return 0;
	} else {
		for_each_net (net) {
			vlan_dev = __dev_get_by_name(net, vlan_name);
			if (vlan_dev) {
				++nl_kserver_error;
				if (vlan_opt->is_up) {
					NL_KSERV_DBG("vlan name=%s is not find vrf IS UP!\n", vlan_name);
					netif_carrier_on(vlan_dev);
				} else {
					NL_KSERV_DBG("vlan name=%s is not find vrf IS DOWN!\n", vlan_name);
					netif_carrier_off(vlan_dev);
				}
				return 0;
			}
		}
	}
	NL_KSERV_ERR("failed vlan is not vrf!\n");

	return NETLINK_SERV_ERR;
}

static void nl_kserver_recv_msg(struct sk_buff *skb)
{
	int pid = 0;
	struct nlmsghdr *nl_hdr = NULL;
	struct reply_with_kernel reply;
	struct sk_buff *skb_to_user = NULL;
	struct request_to_kernel *request = NULL;
	memset(&reply, 0, sizeof(reply));

	nl_hdr = (struct nlmsghdr *)skb->data;
	if (nl_hdr == NULL) {
		NL_KSERV_ERR("failed packet!\n");
		reply.err = NETLINK_SERV_ERR;
		goto out;
	}
	pid = nl_hdr->nlmsg_pid;
	request = (struct request_to_kernel *)nlmsg_data(nl_hdr);
	if (request == NULL) {
		NL_KSERV_ERR("failed request with kernel!\n");
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
	if (reply.err == NETLINK_SERV_ERR)
		++nl_kserver_error;
	else
		++nl_kserver_norm_msg;

	skb_to_user = nlmsg_new(MAX_PAYLOAD, GFP_KERNEL);
	if (skb_to_user == NULL) {
		++nl_kserver_error;
		NL_KSERV_ERR("failed to allocate memory!\n");
		return;
	}
	nl_hdr = nlmsg_put(skb_to_user, 0, 0, NLMSG_DONE, MAX_PAYLOAD, 0);
	NETLINK_CB(skb_to_user).dst_group = 0;
	memcpy((void *)nlmsg_data(nl_hdr), (void *)&reply, sizeof(struct reply_with_kernel));
	if (nlmsg_unicast(nl_socket, skb_to_user, pid) < 0) {
		++nl_kserver_error;
		NL_KSERV_ERR("failed send message to user-space!\n");
	}
}

struct netlink_kernel_cfg cfg = {
	.input = nl_kserver_recv_msg,
};

static ssize_t nl_kserver_read_info(struct file *file, char *buf,
				    size_t count, loff_t *ppos)
{
	static char buf_msg[LEN_MSG];
	memset(buf_msg, 0, LEN_MSG);

	snprintf(buf_msg, LEN_MSG, "%s%lu\n%s%lu\n", "Normal message: ", nl_kserver_norm_msg,
						     "Errors: ", nl_kserver_error);

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

static ssize_t nl_kserver_control(struct file *file, const char *buf,
				  size_t count, loff_t *ppos)
{
	return 0;
}

static const struct file_operations nl_fops = {
	.owner = THIS_MODULE,
	.read = nl_kserver_read_info,
	.write = nl_kserver_control
};

static int __init nl_kserver_init(void)
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

static void __exit nl_kserver_exit(void)
{
	netlink_kernel_release(nl_socket);
	remove_proc_entry(MODULE_NAME, NULL);

	NL_KSERV_ERR("unloaded!\n");
}

module_init(nl_kserver_init);
module_exit(nl_kserver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Netlink server");
MODULE_AUTHOR("Anton Mikaev (ESR Group)");

