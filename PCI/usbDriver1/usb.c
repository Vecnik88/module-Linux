/* 
	Работа с usb устройствами, разбор для ядра 4.4
 */

#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>

struct my_usb_info{
	int connect_count;
};

#define LOG(...) printk( KERN_INFO __VA_ARGS__ )

static int my_usb_probe( struct usb_interface* intf, const struct usb_device_id* id ){
	struct my_usb_info* usb_info;
	struct usb_device* dev = interface_to_usbdev( intf );
	static int my_counter = 0;
	LOG( "connect\n" );
	LOG( "devnum=%d, speed=%d\n", dev->devnum, ( int )dev->speed );
	LOG( "idVendor=0x%hX, idProduct=%0x%hX, bcdDevice=0x%hX\n",
		 dev->descriptor.idVendor, dev->descriptor.idProduct, dev->descriptor.bcdDevice );

	LOG( "class=0x%hX, subclass=0x%hX\n", dev->descriptor.bDeviceClass, dev->descriptor.bDeviceSubClass );
	LOG( "protocol=0x%hX, packetsize=%hu\n", dev->descriptor.bDeviceProtocol, dev->descriptor.bMaxPacketSize0 );
	LOG( "manufacturer=0x%hX, product=0x%hX, serial=%hu\n", dev->descriptor.iManufacturer, 
		 dev->descriptor.iProduct, dev->descriptor.iSerialNumber );

	usb_info = kmalloc( sizeof( struct my_usb_info ), GFP_KERNEL );
	usb_info->connect_count = my_counter++;
	usb_set_intfdata( intf, usb_info );

	LOG( "connect_count=%d\n\n", usb_info->connect_count );

	return 0;
}

static void my_usb_disconnect( struct sub_interface* intf ){
	struct my_usb_info* usb_info;
	usb_info = usb_get_intfdata( intf );
	LOG( "disconnect" );
	kfree( usb_info );
}

static struct usb_device_id my_usb_table[] = {
	{ USB_DEVICE( 0x090c, 0x1000 ) },
	{ }
};

MODULE_DEVICE_TABLE( usb, my_usb_table );

static struct usb_driver my_usb_driver = {
	.name = "usb-my",
	.probe = my_usb_probe,
	.disconnect = my_usb_disconnect,
	.id_table = my_usb_table,
};

static int __init my_init_module( void ){
	int err;
	LOG( "Hello USB\n" );
	err = usb_register( &my_usb_driver );
	return err;
}

static void __exit my_exit_module( void ){
	LOG( "Goodbye USB\n" );
	usb_deregister( &my_usb_driver );
}

module_init( my_init_module );
module_exit( my_exit_module );


