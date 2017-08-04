/* Разработка блочного драйвера :) */

#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/errno.h>
#include <linux/hdreg.h>
#include <linux/version.h>


#define MY_DEVICE_NAME "xd"
#define DEV_MINORS 16

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )
#define ERR(...) printk( KERN_ERR "ERROR " __VA_ARGS__ )
#define DEBUG(...) if( debug > 0 ) printk( KERN_DEBUG "# " __VA_ARGS__ )

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

static int my_getgeo( struct block_device *bdev, struct hd_geometry *geo ) {
   unsigned long sectors = ( diskmb * 1024 ) * 2;
   DBG( KERN_INFO "getgeo\n" );
   geo->heads = 4;
   geo->sectors = 16;
   geo->cylinders = sectors / geo->heads / geo->sectors;
   geo->start = geo->sectors;
   return 0;
};

static int my_ioctl( struct block_device *bdev, fmode_t mode,
                     unsigned int cmd, unsigned long arg ) {
   LOG( "ioctl cmd=%X\n", cmd );
   switch( cmd ) {
      case HDIO_GETGEO: {
         struct hd_geometry geo;
         LOG( "ioctk HDIO_GETGEO\n" );
         my_getgeo( bdev, &geo );
         if( copy_to_user( (void __user *)arg, &geo, sizeof( geo ) ) )
            return -EFAULT;
         return 0;
      }
      default:
         ERR( "ioctl unknown command\n" );
         return -ENOTTY;
   }
}

static void setup_device( struct disk_dev* dev, int which ){
	memset( dev, 0, sizeof( struct disk_dev ) );
	dev->size = diskmb * 1024 * 1024;
	dev->data = vmalloc( dev->size );
	if( dev->data == NULL ){
		ERR( "vmalloc failure. \n" );
		return;
	}

	spin_lock_init( &dev->lock );
	switch( mode ){
		case RM_NOQUEUE:
			dev->queue = blk_alloc_queue( GFP_KERNEL );
			if( dev->queue == NULL )
				goto out_vfree;
			blk_queue_make_request( dev->queue, make_request );
			break;

		case RM_FULL:
			dev->queue = blk_init_queue( full_request, &dev_lock );
			if( dev->queue == NULL )
				goto out_vfree;
			break;

		default:
		LOG( "bad request mode %d, using simple\n", mode );

		case RM_SIMPLE:
		dev->queue = blk_init_queue( simple_request, &dev->lock );
		if( dev->queue == NULL )
			goto out_vfree;
		break;
	}

	blk_queue_logical_block_size( dev->queue, hardsect_size );
	dev->queue->queuedata = dev;
	dev->gd = allock_disk( DEV_MINORS );							// <---. число разделов при разбиении
}

