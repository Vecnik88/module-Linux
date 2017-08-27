#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>

#define SHARED_IRQ 17
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;
module_param( irq, int, S_IRUGO );

static irqreturn_t my_interrupt( int irq, void* dev_id ){
	++irq_counter;
	LOG( "In the ISR: counter = %d\n", irq_counter );

	return IRQ_NONE;
}

static int __init my_init( void ){
	if( request_irq( irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id ) )
		return -1;

	LOG( "Successfully loading ISR handler on IRQ %d\n", irq );

	return 0;
}

static void __exit my_exit( void ){
	synchronize_irq( irq );
	free_irq( irq, &my_dev_id );

	LOG( "Successfully unloading, irq_counet = %d\n", irq_counter );
}

module_init( my_init );
module_exit( my_exit );

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Vecnik88" );
MODULE_DESCRIPTION( "HO HO HO Test module practik" );



