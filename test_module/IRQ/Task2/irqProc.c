/*
		Установите свой дополнительный обработчик прерываний (на любой IRQ, например сетевой карты),
	численное значение IRQ указывайте параметром загрузки модуля. Oтображайте динамически общее число обслуженных
	прерываний в собственное путевое имя в /proc .
*/

#include <linux/init.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>

#define BUF_PROC 20										/* размер буфера файла /proc */
#define NUMBER_IRQ 18									/* номер линии, на которой будем отслеживать прерывания */
#define ERR(...) printk( KERN_ERR __VA_ARGS__ )
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static int irq = NUMBER_IRQ;
static int my_dev_id;
static int irq_counter = 0;

char messageProc[BUF_PROC] = "CountIRQ = 0\n";

module_param( irq, int, S_IRUGO );

static irqreturn_t Handler( int irq, void* dev_id ){
	++irq_counter;
	sprintf( messageProc, "CountIRQ = %d\n", irq_counter );

	return IRQ_NONE;
}

ssize_t procRead( struct file* filp, char* buf, size_t count, loff_t* offp ){
	int temp = strlen( messageProc );
	if( *offp > strlen( messageProc ) ){
		ERR( "Error read loff_t" );
		return -ENOMEM;
	}
	if( ( count + *offp ) > temp )
		count = temp - *offp;

	copy_to_user( buf, messageProc + *offp, count );

	*offp += count;
	LOG( "Read count %d\n", count );

	return count;
}

struct file_operations fops = {							/* Создаем файл в /proc доступный только для чтения */
	.owner = THIS_MODULE,
	.read = procRead,
};

static int __init myIrqInit( void ){
	int res = 0;
	proc_create( "IRQ_counter", 0, NULL, &fops );		/* Создаем файл /proc */
	res = request_irq( irq, Handler, IRQF_SHARED , "Test_Programm", &my_dev_id );
	if( res != 0 ){
		ERR( "Error: request_irq\n" );
		return -1;
	}

	LOG( "========== MODULE IRQPROC VERSION 1.1 LOAD ==========\n" );
	return 0;
} 

static void __exit myIrqExit( void ){
	remove_proc_entry( "IRQ_counter", NULL );			/* Удаляем файл /proc */
	synchronize_irq( irq );
	free_irq( irq, &my_dev_id );

	LOG( "========== MODULE IRQPROC VERSION 1.1 UNLOAD ==========\n" );
}

module_init( myIrqInit );
module_exit( myIrqExit );

MODULE_LICENSE( "GPL" );
MODULE_VERSION( "1.1" );
MODULE_AUTHOR( "Vecnik88" );
MODULE_DESCRIPTION( "Тестовая программа для ядра 4.4.0" );