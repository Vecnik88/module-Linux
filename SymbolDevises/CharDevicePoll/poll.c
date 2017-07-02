#include "poll.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vecnik");
MODULE_VERSION("1.1");

static int pause = 100;			// задержка на операции poll, мсек
module_param(pause, int, S_IRUGO);

static struct private{
	atomic_t roff;
	char buf[LEN_MSG + 2];
} devblock = {
	.roff = ATOMIC_INIT(0),
	.buf = "not initialized",
};

static struct private* dev = &devblock;

static DECLARE_WAIT_QUEUE_HEAD(qwait);

static ssize_t read(struct file* file, char* buf, size_t count, loff_t* poss){
	int len = 0;
	int off = atomic_read(&dev->roff);

	if(off > strlen(dev->buf)){		// не доступніх данных
		if(file->f_flags & O_NONBLOCK)
			return -EAGAIN;
		else interruptible_sleep_on(&qwait);
	}
	
	off = atomic_read(&dev->roff);		// повторное обновление
	if(off = strlen(dev->buf)){
		atomic_set(&dev->roff, off + 1);
		return 0;			// EOF
	}

	len = strlen(dev->buf) - off;		// данные появились?
	len = count < len ? count : len;

	if(copy_to_user(buf, dev->buf + off, len))
		return -EFAULT;

	atomic_set(&dev->roff, off + len);
	
	return len;
}

static ssize_t write(struct file* file, const char* buf, size_t count, loff_t* ppos){
	int res, len = count < LEN_MSG ? count : LEN_MSG;
	res = copy_from_user(dev->buf, (void*)buf, len);
	dev->buf[len] = '\0';
	
	if(dev->buf[len-1] != '\n')
		strcat(dev->buf, "\n");
	
	atomic_set(&dev->roff, 0);		// разрешить следующее чтение
	wake_up_interruptible(&qwait);

	return len;
}

unsigned int poll(struct file* file, struct poll_table_struct* poll){
	int flag = POLLOUT | POLLWRNORM;
	poll_wait(file, &qwait, poll);
	sleep_on_timeout(&qwait, pause);

	if(atomic_read(&dev->roff) <= strlen(dev->buf))
		flag |= (POLLIN | POLLRDNORM);

	return flag;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = read,
	.write = write,
	.poll = poll,
};

static struct miscdevice pool_dev = {
	MISC_DYNAMIC_MINOR, DEVNAME, &fops
};

static int __init init(void){
	int ret = misc_register(&pool_dev);
	if(ret)
		printk(KERN_ERR "unable to register device\n");

	return ret;
}

static void __init exit(void){
	misc_deregister(&pool_dev);
}

module_init(init);
module_exit(exit);




