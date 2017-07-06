#include "device.h"

static int major = 0;	// старший номер устройства
module_param(major, int, S_IRUGO);

static int mode = 0;	// открытие устройства: 0 - без контроля, 1 - единичное, 2 - множественное
module_param(mode, int, S_IRUGO);

static int dev_open = 0;




static int open_dev(struct inode* n, struct file* f){
	LOG("open - node: %p, file: %p, refcount: %d", n, f, module_refcount(THIS_MODULE));


	if(dev_open){
		LOG("device /dev/%s is busy", DEV_NAME);
		return -EBUSY;
	}

	/*switch(mode){
		case 1:
			++dev_open;
			break;

		case 2:{
			struct device_data* data;
			f->private_data = kmalloc(sizeof(struct device_data), GFP_KERNEL);
			if(f->private_data == NULL){
				LOG("memory allocation error");
				return -ENOMEM;
			}
			data = (struct device_data*)f->private_data;
			strcpy(data->buf, "dynamic: not initialized");		// динамический буфер
			data->odd = 0;
			break;
		}	
		default:
			break;
	}*/
	if(mode == 1){
		++dev_open;
	}

	if(mode == 2){
		struct device_data* data;
		f->private_data = kmalloc(sizeof(struct device_data), GFP_KERNEL);
		if(f->private_data == NULL){
			LOG("memory allocation error");
			return -ENOMEM;
		}
		data = (struct device_data*)f->private_data;
		strcpy(data->buf, "dynamic: not initialized");		// динамический буфер
		data->odd = 0;
	}

	return 0;
}

static int release_dev(struct inode* n, struct file* f){
	LOG("close - node: %p, file: %p, refcount: %d", n, f, module_refcount(THIS_MODULE));

	switch(mode){
		case 1:
			--dev_open;
			break;

		case 2:
			kfree(f->private_data);
			break;

		default:
			break;
	}

	return 0;
}

static struct device_data* get_buffer(struct file* f){
	static struct device_data static_buf = {"static: not initialized!", 0};		// статический буфер

	return (mode == 2) ? (struct device_data*)f->private_data : &static_buf;
}

static ssize_t read_dev(struct file* f, char* buf, size_t count, loff_t* pos){
	struct device_data* data = get_buffer(f);
	LOG("read - file: %p, read from %p bytes %d; refcount: %d", f, data, count, module_refcount(THIS_MODULE));

	if(data->odd < 0){
		int res = copy_to_user((void*)buf, data->buf, strlen(data->buf));
		data->odd = 1;
		put_user('\n', buf + strlen(data->buf));
		res = strlen(data->buf)+1;
		LOG("return bytes: %d", res);
		return res;
	}	

	data->odd = 0;
	LOG("return : EOF");
	return 0;
}

static ssize_t write_dev(struct file* f, const char* buf, size_t count, loff_t* pos){
	int res, len = (count < BUF_SIZE) ? count : BUF_SIZE;
	
	struct device_data* data = get_buffer(f);

	LOG("write - file: %p, write to %p bytes %d; refcount: %d", f, data, count, module_refcount(THIS_MODULE));
	res = copy_from_user(data->buf, (void*)buf, len);
	
	if(data->buf[len - 1] == '\n')
		data->buf[len - 1] = '\0';
	else data->buf[len] = '\0';

	LOG("put bytes : %d", len);

	return len;
}

static const struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.open = open_dev,
	.release = release_dev,
	.read = read_dev,
	.write = write_dev,
};

static struct cdev hcdev;
static struct class* devclass;

static int __init dev_init(void){
	int ret, i;
	dev_t dev;

	if(major != 0){
		dev = MKDEV(major, DEVICE_FIRST);
		ret = register_chrdev_region(dev, DEVICE_COUNT, DEV_NAME);
	} else {
		ret = alloc_chrdev_region(&dev, DEVICE_FIRST, DEVICE_COUNT, DEV_NAME);
		major = MAJOR(dev);
	}

	if(ret < 0){
		ERR("===== Can not register char device region\n =====");
		goto err;
	}

	cdev_init(&hcdev, &dev_fops);
	hcdev.owner = THIS_MODULE;
	ret = cdev_add(&hcdev, dev, DEVICE_COUNT);

	if(ret < 0){
		unregister_chrdev_region(MKDEV(major, DEVICE_FIRST), DEVICE_COUNT);
		ERR("===== Can not add char device \n =====");
		goto err;
	}
	
	devclass = class_create(THIS_MODULE, "dyn_class");

	for(i = 0; i < DEVICE_COUNT; ++i){
		dev = MKDEV(major, DEVICE_FIRST + i);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,26)
	device_create(devclass, NULL, dev, "%s_%d", DEV_NAME, i);
#else
	device_create(devclass, NULL, dev, NULL, "%s_%d", DEV_NAME, i);
#endif
	}

	LOG("===== module init =====\n");
	
	return 0;

err:
	return ret;
}


static void __exit dev_exit(void){
	dev_t dev;
	int i;
	for(i = 0; i < DEVICE_COUNT; ++i){
		dev = MKDEV(major, DEVICE_FIRST + i);
		device_destroy(devclass, dev);
	}

	class_destroy(devclass);
	cdev_del(&hcdev);
	unregister_chrdev_region(MKDEV(major, DEVICE_FIRST), DEVICE_COUNT);
	LOG("===== module removed =====\n");
}
