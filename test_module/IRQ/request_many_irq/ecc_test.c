#include <linux/module.h>
#include <linux/interrupt.h>

#define SHARED_IRQ 1
#define MAX_SHARED 9
#define NAME_SUFFIX "serial-"
#define NAME_LEN 10

static int irq = SHARED_IRQ, num = 2;

module_param(irq, int, 0);
module_param(num, int, 0);

static irqreturn_t handler(int irq, void *id)
{
	cycles_t cycles = get_cycles();
	printk("%010lld : irq=%d - handler #%d\n", cycles, irq, (int)id);

	return IRQ_NONE;
}

static char dev[MAX_SHARED][NAME_LEN];

static int __init ecc_test_module_init(void)
{
	int i;
	if (num > MAX_SHARED)
		num = MAX_SHARED;
	for (i = 0; i < num; ++i) {
		sprintf(dev[i], "serial_%02d", i + 1);
		if (request_irq(irq, handler, IRQF_SHARED, dev[i], (void *)(i + 1)))
			return -1;
	}
	return 0;
}

static void __exit ecc_test_module_exit(void)
{
	int i;
	for (i = 0; i < num; ++i) {
		synchronize_irq(irq);
		free_irq(irq, (void *)(i + 1));
	}
}

module_init(ecc_test_module_init);
module_exit(ecc_test_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anton Mikaev");
MODULE_DESCRIPTION("Test module with interrupt");