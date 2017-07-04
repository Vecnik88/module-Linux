/*

		Напишите модуль, который перехватывает icmp-request пакеты 
	и формирует свой ответ на них (icmp-reply), без дальнейшего отправления icm-request. 

*/

#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vecnik88");
MODULE_DESCRIPTION("Test programm");

static int myInit( void );
static void myExit( void );
static unsigned int myHookFunction(void* priv, struct sk_buff* skb, const struct nf_hook_state* state);

module_init(myInit);
module_exit(myExit);