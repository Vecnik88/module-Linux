#include "icmp.h"

static unsigned int myHookFunction(void* priv, struct sk_buff* skb, const struct nf_hook_state* state){

	struct ethhdr* eth;
	struct iphdr* ip;
	struct icmphdr* icmp;

	struct sk_buff* skbSendReply;
	struct ethhdr* ethSendReply;
	struct iphdr* ipSendReply;
	struct icmphdr* icmpSendReply;

	unsigned int skbSize;
	unsigned int icmpSize;

	eth = eth_hdr(skb);
	ip = ip_hdr(skb);

	/* Делаем разбор пакета - если он не ICMP и не запрос на соединение, то пропускаем дальше */
	if(ip->protocol == IPPROTO_ICMP){
		icmp = icmp_hdr(skb);
		if(icmp->type != ICMP_ECHO)
				goto send_packet;
	} else {
		goto send_packet;
	}

	icmpSize = ntohs(ip->tot_len) - sizeof(struct iphdr);
	skbSize = ntohs(ip->tot_len) + sizeof(struct ethhdr);

	// формируем новый пакет
	skbSendReply = alloc_skb(skbSize, GFP_ATOMIC);
	if(skbSendReply == NULL){
		printk(KERN_ERR "Error: alloc_skb. Packet accept");
		goto send_packet;
	}

	skbSendReply->sk = skb->sk;			// сокет, через который осуществляется связь
	skbSendReply->dev = state->in;		// сетевое устройство, на которое пришел пакет

	// начинаем заполнение нашей стуктуры
	skb_set_mac_header(skbSendReply, 0);
	ethSendReply = eth_hdr(skbSendReply);
	memcpy(ethSendReply, eth, sizeof(struct ethhdr));
	memcpy(ethSendReply->h_dest, eth->h_source, ETH_ALEN);
	memcpy(ethSendReply->h_source, eth->h_dest, ETH_ALEN);

	skb_set_network_header(skbSendReply, sizeof(struct ethhdr));
	ipSendReply = ip_hdr(skbSendReply);
	memcpy(ipSendReply, ip, sizeof(struct iphdr));
	ipSendReply->saddr = ip->daddr;
	ipSendReply->daddr = ip->saddr;
	ipSendReply->ttl = 64;
	ipSendReply->check = 0;
	ipSendReply->tot_len = htons(sizeof(struct iphdr) + icmpSize);
	ipSendReply->check = ip_compute_csum((void *)ipSendReply, sizeof(struct iphdr));

	skb_set_transport_header(skbSendReply, sizeof(struct iphdr) + sizeof(struct icmphdr));
	icmpSendReply = icmp_hdr(skbSendReply);
	memcpy(icmpSendReply, icmp, icmpSize);
	icmpSendReply->type = ICMP_ECHOREPLY;
	icmpSendReply->checksum = 0;
	icmpSendReply->checksum = ip_compute_csum((void *)icmpSendReply, icmpSize);
	printk(KERN_INFO "Message send\n");
	dev_queue_xmit(skbSendReply);

	return NF_DROP;						// <---. блокируем дальнейшее продвижение этого пакета

send_packet:
	return NF_ACCEPT;
}

static struct nf_hook_ops myHookStruct;

static int myInit( void ){
	myHookStruct.hook = myHookFunction;
	myHookStruct.pf = PF_INET;
	myHookStruct.hooknum = NF_INET_LOCAL_IN;
	myHookStruct.priority = NF_IP_PRI_FIRST;
	nf_register_hook( &myHookStruct );
	printk( KERN_INFO "========== MODULE HOOK FUNCTION INIT ==========\n" );

	return 0;
}

static void myExit( void ){
	nf_unregister_hook( &myHookStruct );
	printk( KERN_INFO "========== MODULE HOOK FUNCTION EXIT ==========\n" );
}


