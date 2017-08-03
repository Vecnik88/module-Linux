/* Разработка блочного драйвера :) */

#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/errno.h>
#include <linux/hdreg.h>
#include <linux/version.h>

static int diskmb = 4;
module_param_named( size, diskmb, int, 0 );				// <---. размер диска в Mb

static int debug = 0;
module_param( debug, int, 0 );							// <---. для отладки

static int major = 0;									// <---. старший номер устройства
module_param( major, int, 0 );

static int hardsect_size = KERNEL_SECTOR_SIZE;
module_param( hardsect_size, int, 0 );

static int ndevices = 4;
module_param( ndevices, int, 0 );

enum { RM_SIMPLE = 0, RM_FULL = 1, RM_NOQUEUE = 2 };

staticint mode = RM_SIMPLE;
module_param( mode, int, 0 );

static int nsectors;

struct disk_dev{
	int size;
	u8 * data;
	spinlock_t lock;
	struct request_queue * queue;
	struct gendisk * gd;
};

static struct disk_dev* Devices = NULL;

static void simple_request( struct request_queue * q ){

}

static void full_request( struct request_queue* q ){

}

static void make_request( struct request_queue * q, struct bio * bio ){

}

#define MY_DEVICE_NAME "xd"
#define DEV_MINORS 16

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )
#define ERR(...) printk( KERN_ERR "ERROR " __VA_ARGS__ )
#define DEBUG(...) if( debug > 0 ) printk( KERN_DEBUG "# " __VA_ARGS__ )