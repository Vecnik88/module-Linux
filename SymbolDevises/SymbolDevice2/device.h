/*	Created by Vecnik88 2.07.2017
	Символьный драйвер, который имеет возможность читать и писать.
	Реализован в виде практики.
*/

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vecnik88");
MODULE_VERSION("1.1");
MODULE_DESCRIPTION("Test programm. A complete symbol module");

#define LOG(...) printk(KERN_INFO "! " __VA_ARGS__)
#define ERR(...) printk(KERN_ERR "! " __VA_ARGS__)
#define DEV_NAME "test_symbol_device"
#define BUF_SIZE 1024
#define DEVICE_FIRST 0
#define DEVICE_COUNT 3

struct device_data {                    // область данных драйвера
        char buf[BUF_SIZE + 1];         // буфер данных
        int odd;                        // признак начала чтения
};

static int __init dev_init(void);
static void __init dev_exit(void);

module_init(dev_init);
module_exit(dev_exit);
\\\
