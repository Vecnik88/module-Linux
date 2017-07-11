/*
		Установите свой дополнительный обработчик прерываний (на любой IRQ, например сетевой карты),
	численное значение IRQ указывайте параметром загрузки модуля. Модуль должен диагностировать
	полученные прерывания в системный журнал и подсчитывать их общее число, диагностируя его при
	выгрузке модуля

*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>

#define NUMBER_IRQ 17						/* номер аппаратной линии прерываний по умолчанию */
#define ERR(...) printk( KERN_ERR __VA_ARGS__ )
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static int my_dev_id = 178;					/* id номера устр-ва, лучше указывать ссылку 
												на структуру созданную самому, но можно и так */
static int irq_counter = 0;					/* счетчик срабатываний прерываний */
static int irq = NUMBER_IRQ;					/* линия, на которой будемотслеживать количество прерываний */
module_param( irq, int, S_IRUGO );

static irqreturn_t functionHandlerIRQ( int irq, void* dev_id ){	/* наша фун-ция обработки прерываний */
	++irq_counter;
	LOG( "IRQ( ISR ) - work\n" );

	return IRQ_NONE;					/* пропускаем, говорим что это не мы должны его обрабатывать */
}

static int __init irq_initH( void ){
	int res = 0;
	res = request_irq(  irq, functionHandlerIRQ, IRQF_SHARED, "my_handler", &my_dev_id );
	if( res != 0 ){
		ERR( "Error: request_irq\n" );
		return -1;
	}

	LOG( "========== MODULE IRQ VERSION 1.1 LOAD ==========\n" );

	return 0;
}

static void __exit irq_exitH( void ){
	synchronize_irq( irq );
	free_irq( irq, &my_dev_id );

	LOG( "Number of interrupt handlers %d\n", irq_counter );
	LOG( "========== MODULE IRQ VERSION 1.1 UNLOAD ==========\n" );
}

module_init( irq_initH );
module_exit( irq_exitH );

MODULE_LICENSE( "GPL" );
MODULE_VERSION( "1.1" );
MODULE_AUTHOR( "Vecnik88" );
MODULE_DESCRIPTION( "My experiments with IRQ" );
