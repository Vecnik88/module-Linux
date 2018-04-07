#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#define MODULE_NAME "policing"

#define POLICING_LOG(fmt, ...)  	   \
	printk(KERN_INFO MODULE_NAME ": "  \
	       "[%s] " fmt "!\n",          \
	       __func__, ##__VA_ARGS__)

#define POLICING_ERR(fmt, ...)  	   \
	printk(KERN_ERR MODULE_NAME ": "   \
	       "[%s] " fmt "!\n",          \
	       __func__, ##__VA_ARGS__)

/* Info:
 *
 * For speeds less than 1 Mbit/s, we recommend burst = cir/8*1.5.
 * Otherwise, the explosion = cir/8*0,25.
 * Cir is rate.
 * Burst is given in byte.
 * Rate is given in bit/s.
 */

/*
 * The maximum permissible speed at
 * session/interface/service in bits/second
 */
#define RATE_IN  50000000

/* Maximum packet size in BYTEs */
#define BURST_IN 4166666

u_int32_t tokens_in = 0;
u_int32_t rate_in = RATE_IN;
u_int32_t burst_in = BURST_IN;
u_int64_t last_seen_in = 0;

static void update_tokens(void)
{
	u_int64_t tokens, now;
	struct timespec ts_now;

	ktime_get_ts(&ts_now);

	now = timespec_to_ns(&ts_now);

	tokens = div_s64(rate_in * (now - last_seen_in), NSEC_PER_SEC);

	if ((tokens_in + tokens) > burst_in)
		tokens_in = burst_in;
	else
		tokens_in += tokens;

	last_seen_in = now;
}

static inline int32_t checking_sess(u_int32_t pkt_len)
{
	u_int32_t pkt_len_bits = pkt_len << 3;

	update_tokens();
	if (pkt_len_bits <= tokens_in || !rate_in)
	{
		tokens_in -= pkt_len_bits;

		return NF_ACCEPT;
	}
	return NF_DROP;
}

static unsigned int hook_func(      void                 *priv,
                                    struct sk_buff       *skb,
                              const struct nf_hook_state *state)
{
	unsigned int err = NF_ACCEPT;

	if ((err = checking_sess(ntohs(ip_hdr(skb)->tot_len))) != NF_ACCEPT) {
		POLICING_LOG("packet dropped");
		goto exit;
	}

exit:
	return err;
}

static struct nf_hook_ops hook_opt __read_mostly  = {
	.hook = hook_func,
	.pf = PF_INET,
	.hooknum = NF_INET_LOCAL_IN,
	.priority = NF_IP_PRI_FIRST,
};

static int __init start_module(void)
{
	int err = 0;

	err = nf_register_hook(&hook_opt);
	if (unlikely(err < 0))
		goto exit;

	POLICING_LOG("start module");

exit:
	return err;
}

static void __exit exit_module(void)
{
	nf_unregister_hook(&hook_opt);

	POLICING_LOG("exit module");

}

module_init(start_module);
module_exit(exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anton Mikaev");
MODULE_DESCRIPTION("A simple module for studying the work"
                   "of limiting the speed of traffic per user");
