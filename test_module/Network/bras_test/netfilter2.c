#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/icmp.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/proc_ns.h>
#include <linux/proc_fs.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include "netfilter2.h"

#define BRAS_TEST_LOG(fmt, ...)  	   \
	printk(KERN_INFO MODULE_NAME ": "  \
	       "[%s] " fmt "\n",           \
	       __func__, ##__VA_ARGS__)

#define BRAS_TEST_ERR(fmt, ...)  	   \
	printk(KERN_ERR MODULE_NAME ": "   \
	       "[%s] " fmt "\n",           \
	       __func__, ##__VA_ARGS__)

#define BRAS_TEST_DBG(fmt, ...)  	       \
	if (unlikely(bras_test_debug))         \
	    printk(KERN_ALERT MODULE_NAME ": " \
	           "[%s] " fmt "\n",           \
	           __func__, ##__VA_ARGS__)

int bras_test_debug = 1;
module_param_named(debug, bras_test_debug, int, 0644);
MODULE_PARM_DESC(debug, "Debuging this module: set (0=off debug, 1=on debug)");

/* stats testing module */
u64 ipproto_ip = 0;
u64 ipproto_tp = 0;
u64 ipproto_ah = 0;
u64 ipproto_mtp = 0;
u64 ipproto_tcp = 0;
u64 ipproto_egp = 0;
u64 ipproto_pup = 0;
u64 ipproto_udp = 0;
u64 ipproto_idp = 0;
u64 ipproto_gre = 0;
u64 ipproto_esp = 0;
u64 ipproto_pim = 0;
u64 ipproto_raw = 0;
u64 ipproto_max = 0;
u64 ipproto_sctp = 0;
u64 ipproto_comp = 0;
u64 ipproto_icmp = 0;
u64 ipproto_igmp = 0;
u64 ipproto_ipip = 0;
u64 ipproto_dccp = 0;
u64 ipproto_rsvp = 0;
u64 ipproto_ipv6 = 0;
u64 ipproto_mpls = 0;
u64 ipproto_encap = 0;
u64 ipproto_udplite = 0;
u64 ipproto_default = 0;

u64 ipv6_protocols = 0;
u64 ipv4_protocols = 0;

static unsigned int bras_test_hook_ipv4(void                       *priv,
                                        struct sk_buff             *skb,
                                        const struct nf_hook_state *state)
{
	struct iphdr *ip = NULL;
	struct ethhdr *eth = NULL;

	eth = eth_hdr(skb);
	ip = ip_hdr(skb);

	++ipv4_protocols;

	BRAS_TEST_DBG("eth->protocol=0x%x", eth->h_proto);

	switch (ip->protocol) {
		case IPPROTO_IP:
			++ipproto_ip;
			BRAS_TEST_DBG("recv IPPROTO_IP");
			break;
		case IPPROTO_ICMP:
			++ipproto_icmp;
			BRAS_TEST_DBG("recv IPPROTO_ICMP");
			break;
		case IPPROTO_IGMP:
			++ipproto_igmp;
			BRAS_TEST_DBG("recv IPPROTO_IGMP");
			break;
		case IPPROTO_IPIP:
			++ipproto_ipip;
			BRAS_TEST_DBG("recv IPPROTO_IPIP");
			break;
		case IPPROTO_TCP:
			++ipproto_tcp;
			BRAS_TEST_DBG("recv IPPROTO_TCP");
			break;
		case IPPROTO_EGP:
			++ipproto_egp;
			BRAS_TEST_DBG("recv IPPROTO_EGP");
			break;
		case IPPROTO_PUP:
			++ipproto_pup;
			BRAS_TEST_DBG("recv IPPROTO_PUP");
			break;
		case IPPROTO_UDP:
			++ipproto_udp;
			BRAS_TEST_DBG("recv IPPROTO_UDP");
			break;
		case IPPROTO_IDP:
			++ipproto_idp;
			BRAS_TEST_DBG("recv IPPROTO_IDP");
			break;
		case IPPROTO_TP:
			++ipproto_tp;
			BRAS_TEST_DBG("recv IPPROTO_TP");
			break;
		case IPPROTO_DCCP:
			++ipproto_dccp;
			BRAS_TEST_DBG("recv IPPROTO_DCCP");
			break;
		case IPPROTO_IPV6:
			++ipproto_ipv6;
			BRAS_TEST_DBG("recv IPPROTO_IPV6");
			break;
		case IPPROTO_RSVP:
			++ipproto_rsvp;
			BRAS_TEST_DBG("recv IPPROTO_RSVP");
			break;
		case IPPROTO_GRE:
			++ipproto_gre;
			BRAS_TEST_DBG("recv IPPROTO_GRE");
			break;
		case IPPROTO_ESP:
			++ipproto_esp;
			BRAS_TEST_DBG("recv IPPROTO_ESP");
			break;
		case IPPROTO_AH:
			++ipproto_ah;
			BRAS_TEST_DBG("recv IPPROTO_AH");
			break;
		case IPPROTO_MTP:
			++ipproto_mtp;
			BRAS_TEST_DBG("recv IPPROTO_MTP");
			break;
		case IPPROTO_BEETPH:
			++ipproto_mtp;
			BRAS_TEST_DBG("recv IPPROTO_BEETPH");
			break;
		case IPPROTO_ENCAP:
			++ipproto_encap;
			BRAS_TEST_DBG("recv IPPROTO_ENCAP");
			break;
		case IPPROTO_PIM:
			++ipproto_pim;
			BRAS_TEST_DBG("recv IPPROTO_PIM");
			break;
		case IPPROTO_COMP:
			++ipproto_comp;
			BRAS_TEST_DBG("recv IPPROTO_COMP");
			break;
		case IPPROTO_SCTP:
			++ipproto_sctp;
			BRAS_TEST_DBG("recv IPPROTO_SCTP");
			break;
		case IPPROTO_UDPLITE:
			++ipproto_udplite;
			BRAS_TEST_DBG("recv IPPROTO_UDPLITE");
			break;
		case IPPROTO_MPLS:
			++ipproto_mpls;
			BRAS_TEST_DBG("recv IPPROTO_MPLS");
			break;
		case IPPROTO_RAW:
			++ipproto_raw;
			BRAS_TEST_DBG("recv IPPROTO_RAW");
			break;
		case IPPROTO_MAX:
			++ipproto_max;
			BRAS_TEST_DBG("recv IPPROTO_MAX");
			break;
		default:
			++ipproto_default;
			BRAS_TEST_DBG("default error message %d", ip->protocol);
	}

	return NF_ACCEPT;
}

static unsigned int bras_test_hook_ipv6(void                       *priv,
                                        struct sk_buff             *skb,
                                        const struct nf_hook_state *state)
{
	struct ethhdr *eth = NULL;
	struct ipv6hdr *ipv6 = NULL;

	eth = eth_hdr(skb);
	ipv6 = ipv6_hdr(skb);

	++ipv6_protocols;

	return NF_ACCEPT;
}

static const struct nf_hook_ops bras_test_nf_opt[] = {
	{
		.hook = bras_test_hook_ipv4,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_FIRST,
	},
	{
		.hook = bras_test_hook_ipv6,
		.pf = NFPROTO_IPV6,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_FIRST,
	}
};

void bras_test_get_stats(char *buf_msg)
{
	snprintf(buf_msg, LEN_MSG, "%s%llu\n%s%llu\n%s%llu\n"
	                           "%s%llu\n%s%llu\n%s%llu\n"
	                           "%s%llu\n%s%llu\n%s%llu\n"
	                           "%s%llu\n%s%llu\n%s%llu\n"
	                           "%s%llu\n%s%llu\n%s%llu\n"
	                           "%s%llu\n%s%llu\n%s%llu\n"
	                           "%s%llu\n%s%llu\n%s%llu\n"
	                           "%s%llu\n%s%llu\n%s%llu\n"
	                           "%s%llu\n%s%llu\n%s%llu\n"
	                           "%s%llu\n%s%llu\n%s%llu\n"
	                           "%s%llu\n%s%llu\n%s%llu\n",
	                           "ipproto_ip = ",      ipproto_ip,
	                           "ipproto_icmp = ",    ipproto_icmp,
	                           "ipproto_igmp = ",    ipproto_igmp,
	                           "ipproto_ipip = ",    ipproto_ipip,
	                           "ipproto_tcp = ",     ipproto_tcp,
	                           "ipproto_egp = ",     ipproto_egp,
	                           "ipproto_pup = ",     ipproto_pup,
	                           "ipproto_udp = ",     ipproto_udp,
	                           "ipproto_idp = ",     ipproto_idp,
	                           "ipproto_tp = ",      ipproto_tp,
	                           "ipproto_dccp = ",    ipproto_dccp,
	                           "ipproto_rsvp = ",    ipproto_rsvp,
	                           "ipproto_ipv6 = ",    ipproto_ipv6,
	                           "ipproto_gre = ",     ipproto_gre,
	                           "ipproto_esp = ",     ipproto_esp,
	                           "ipproto_ah = ",      ipproto_ah,
	                           "ipproto_mtp = ",     ipproto_mtp,
	                           "ipproto_encap = ",   ipproto_encap,
	                           "ipproto_pim = ",     ipproto_pim,
	                           "ipproto_sctp = ",    ipproto_sctp,
	                           "ipproto_comp = ",    ipproto_comp,
	                           "ipproto_udplite = ", ipproto_udplite,
	                           "ipproto_encap = ",   ipproto_encap,
	                           "ipproto_pim = ",     ipproto_pim,
	                           "ipproto_sctp = ",    ipproto_sctp,
	                           "ipproto_comp = ",    ipproto_comp,
	                           "ipproto_udplite = ", ipproto_udplite,
	                           "ipproto_mpls = ",    ipproto_mpls,
	                           "ipproto_raw = ",     ipproto_raw,
	                           "ipproto_max = ",     ipproto_max,
	                           "ipproto_default = ", ipproto_default,
	                           "ipv6_protocols = ",  ipv6_protocols,
	                           "ipv4_protocols = ",  ipv4_protocols);
}

static ssize_t bras_test_read(struct file *file,
                              char __user *buf,
                              size_t       count,
                              loff_t      *ppos)
{
	static char buf_msg[LEN_MSG];
	memset(buf_msg, 0, LEN_MSG);

	bras_test_get_stats(buf_msg);
	if (*ppos >= strlen(buf_msg)) {
		*ppos = 0;
		return 0;
	}
	if (count > strlen(buf_msg) - *ppos)
		count = strlen(buf_msg) - *ppos;
	if (copy_to_user((void *)buf, buf_msg + *ppos, count)) {
		BRAS_TEST_ERR("can't copy data to user space");
		return -EFAULT;
	}
	*ppos += count;

	return count;
}
EXPORT_SYMBOL(bras_test_get_stats);

static ssize_t bras_test_write(struct file       *file,
                               const char __user *buf,
                               size_t             count,
                               loff_t            *ppos)
{
	return 0;
}

static const struct file_operations bras_test_fops = {
	.owner = THIS_MODULE,
	.read = bras_test_read,
	.write = bras_test_write
};

static int __init bras_test_init(void)
{
	int err = 0;
	struct net *net = NULL;
	struct proc_dir_entry *bras_test_node = NULL;

	for_each_net (net) {
		err = nf_register_net_hooks(net,
		                            bras_test_nf_opt,
		                            ARRAY_SIZE(bras_test_nf_opt));
		if (err)
			return NOTIFY_BAD;
	}
	bras_test_node = proc_create(MODULE_NAME,
	                             S_IFREG | S_IRUGO | S_IWUGO,
	                             NULL,
	                             &bras_test_fops);
	if (bras_test_node == NULL) {
		BRAS_TEST_ERR("can't create /proc/%s!", MODULE_NAME);
		return -ENOENT;
	}
	BRAS_TEST_LOG("loaded!");

	return err;
}

static void __exit bras_test_exit(void)
{
	struct net *net = NULL;

	for_each_net (net) {
		nf_unregister_net_hooks(net,
		                        bras_test_nf_opt,
		                        ARRAY_SIZE(bras_test_nf_opt));
	}
	remove_proc_entry(MODULE_NAME, NULL);

	BRAS_TEST_LOG("unloaded!");
}

module_init(bras_test_init);
module_exit(bras_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anton Mikaev");
MODULE_DESCRIPTION("Test module for BRAS");
