#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>

#define MAX_SIZE_BUF_VAL 10
#define MODULE_NAME      "kobject"

#define KOBJ_LOG(fmt, ...)      	   \
	printk(KERN_INFO MODULE_NAME ": "  \
	       "[%s] " fmt "!\n",          \
	       __func__, ##__VA_ARGS__)

static int file1;
static int file2;
static int file3;

static ssize_t file1_show(struct kobject        *kobj,
                          struct kobj_attribute *attr,
                          char                  *buf)
{
	return sprintf(buf, "%d\n", file1);
}

static ssize_t file1_store(struct kobject        *kobj,
                           struct kobj_attribute *attr,
                           const char            *buf,
                           size_t                 count)
{
	int err;

	err = kstrtoint(buf, MAX_SIZE_BUF_VAL, &file1);
	if (err < 0)
		return err;

	return count;
}

static struct kobj_attribute file1_attr = __ATTR(file1,
                                                 0664,
                                                 file1_show,
                                                 file1_store);

static ssize_t all_show(struct kobject        *kobj,
                        struct kobj_attribute *attr,
                        char                  *buf)
{
	int var;

	if (strcmp(attr->attr.name, "file2") == 0)
		var = file2;
	else
		var = file3;

	return sprintf(buf, "%d\n", var);
}

static ssize_t all_store(struct kobject        *kobj,
                         struct kobj_attribute *attr,
                         const char            *buf,
                         size_t                 count)
{
	int var, err;
	err = kstrtoint(buf, MAX_SIZE_BUF_VAL, &var);

	if (err < 0)
		return err;

	if (strcmp(attr->attr.name, "file2") == 0)
		file2 = var;
	else
		file3 = var;

	return count;
}

static struct kobj_attribute file2_attr = __ATTR(file2,
                                                 0664,
                                                 all_show,
                                                 all_store);

static struct kobj_attribute file3_attr = __ATTR(file3,
                                                 0664,
                                                 all_show,
                                                 all_store);

static struct attribute *attrs[] = {
	&file1_attr.attr,
	&file2_attr.attr,
	&file3_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *example_kobj;

static int __init kobj_init(void)
{
	int err;

	example_kobj = kobject_create_and_add("example_kobj", kernel_kobj);
	if (!example_kobj)
		return -ENOMEM;

	err = sysfs_create_group(example_kobj, &attr_group);
	if (err)
		kobject_put(example_kobj);

	KOBJ_LOG("loaded");

	return err;
}

static void __exit kobj_exit(void)
{
	KOBJ_LOG("unloaded");

	kobject_put(example_kobj);
}

module_init(kobj_init);
module_exit(kobj_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anton Mikaev");
MODULE_DESCRIPTION("Test kobject");
