/* Модуль блокирующий все пакеты приходящие на компьютер :) */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

MODULE_AUTHOR("noname");
MODULE_DESCRIPTION("ho-ho-ho");
MODULE_LICENSE("GPL");
unsigned int Hook_Func(uint hooknum, struct sk_buff* skb, 
		       const struct net_device* in,
		       const struct net_device* out,
		       int(*okfn) (struct sk_buff*) ){
	
	return NF_DROP;
}
struct nf_hook_ops bundle;

int __init initFunction(void){
	printk(KERN_INFO " Start module shifter\n");
	
// Заполняем структуру для регистрации hook-функции
	/* имя функции которая будет обрабатывать пакеты */
	bundle.hook = Hook_Func;
	/* семейство протоколов обрабатываем */
	bundle.pf = PF_INET;
	/* место в котором будет срабатывать функция */
	bundle.hooknum = NF_INET_PRE_ROUTING;
	/* приоритет нашей функции - самый высокий, иначе мы будем получать пакеты позже */
	bundle.priority = NF_IP_PRI_FIRST;
	/* регистрируем наше структуру */
	nf_register_hook(&bundle);

	printk(KERN_INFO "======== module init ========");

	return 0;
}

void __init exitFunction(void){
	nf_unregister_hook(&bundle);
	printk(KERN_INFO "======== module removed ========\n");
}

module_init(initFunction);
module_exit(exitFunction);
