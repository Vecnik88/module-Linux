/*
	Выводит весь список сетевых интерфейсов в сообщения ядра dmesg | tail
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )





static int __init my_init( void ){
	struct net_device* dev;
	LOG( "Hello: module loaded at 0x%p\n", my_init );
	dev = first_net_device( &init_net );
	LOG( "Hello: dev_base address = 0x%p\n", dev );

	while( dev ){
		LOG( "name = %6s irq = %4d trans_start = %12lu last_rx = %12lu\n",
			 dev->name, dev->irq, dev->trans_start, dev->last_rx );

		dev = next_net_device( dev );
	}

	return -1;
}

module_init( my_init );
