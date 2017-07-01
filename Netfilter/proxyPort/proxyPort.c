#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

MODULE_AUTHOR("Vecnik");
MODULE_DESCRIPTION("ho-ho-ho");
MODULE_LICENSE("GPL");

#define uchar unsigned char
#define ushort unsigned short
#define uint unsigned int

#define PORT_FALSE 7777

uint Hook_Func(uint hooknum, struct sk_buff* skb, const struct net_device* in,
	       const struct net_device* out, int (*okfn)(struct sk_buff*) ){
	struct iphdr* ip;
	struct udphdr* udp;
	struct tcphdr* tcp;

	if(skb->protocol ==  htons(ETH_P_IP)){
		ip = ip_hdr(skb);
//		ip->check = htons(0);

		switch(ip->protocol){
			case IPPROTO_TCP:
				//skb_set_transport_header(skb, ip->ihl * 4);
				tcp = tcp_hdr(skb);
				tcp->dest = htons(PORT_FALSE);
//				tcp->check = htons(0);
				break;
			case IPPROTO_UDP:
				//skb_set_transport_header(skb, ip->ihl * 4);
				//udp = (struct udphr*)skb_transport_header(skb);
				udp = udp_hdr(skb);
				udp->dest = htons(PORT_FALSE);
//				udp->check = htons(0);
				break;
			default:
				break;
		}
	}

	return NF_ACCEPT;
}

struct nf_hook_ops bundle;/* = {
	.hook =(nf_hookfn*) Hook_Func,
	.pf = PF_INET,
	.hooknum = NF_INET_PRE_ROUTING,
	.priority = NF_IP_PRI_FIRST,
};*/

static int funcInit(void){
	bundle.hook = /*(nf_hookfn*)*/ Hook_Func;
	bundle.pf = PF_INET;
	bundle.hooknum = NF_INET_PRE_ROUTING;
	bundle.priority = NF_IP_PRI_FIRST;
	nf_register_hook(&bundle);
	printk(KERN_INFO "========== module init ==========\n");

	return 0;
}

static void funcExit(void){
	nf_unregister_hook(&bundle);
	printk(KERN_INFO "========== module removed ==========\n");
}

module_init(funcInit);
module_exit(funcExit);
