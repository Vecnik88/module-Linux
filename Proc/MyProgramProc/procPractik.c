/*
	Модель работы и API для файловой системы /proc ( procfs ) радикально поменялись с ядра 3.10.
	Сделайте простой модуль, создающий простейшую иерархию в /proc независимо от версии ядра Linux.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <asm/uaccess.h>

MODULE_AUTHOR( "Vecnik88" );
MODULE_LICENSE( "GPL" );
MODULE_DESCRIPTION( "Ho Ho Ho" );

#define BUF_SIZE 512								// <---. размер нашего буфера
#define NAME_DIR "prk_dir_module"					// <---. имя родительской директории
#define NAME_NODE "prk_node_module"					// <---. имя нашего файла
#define LOG(...) printk( KERN_INFO __VA_ARGS__ )	// <---. заносит логи ядра
#define ERR(...) printk( KERN_ERR __VA_ARGS__ )		// <---. заносит ошибки ядра

static char ourBuffer[ BUF_SIZE ] = "Hello world";	// <---. наш буфер

/* вызывается когда читается наш файл*/
static ssize_t practik_read( struct file* f, char __user* buffer, size_t count, loff_t* ppos ){
	LOG( "read: %ld bytes (ppos = %lld)\n", ( long )count, *ppos );
	if(*ppos > strlen( buffer ) ){
		LOG( "EOF" );
		*ppos = 0;
		return 0;
	}

	if( count > ( strlen( ourBuffer ) - *ppos ) )
		count = strlen( ourBuffer ) - *ppos;

	copy_to_user( (void*)buffer, ourBuffer + *ppos, count );
	*ppos += count;
	LOG( "return %ld bytes\n", ( long )count );

	return count;
}

/* вызывается когда что-то пишется в наш файл*/
static ssize_t practik_write( struct file* f, const char __user* buffer, size_t count, loff_t* ppos ){
	uint len = ( ( count + *ppos ) > BUF_SIZE - 1 ) ? ( BUF_SIZE - *ppos ) : count;
	LOG( "write: %ld bytes\n", ( long )len );
	copy_from_user( ourBuffer + *ppos, ( void* )buffer, len );
	ourBuffer[ len + *ppos ] = '\0';
	LOG( "put %d bytes\n", len );

	return len;
}

/* структура операций над нашим файлом */
static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = practik_read,
	.write = practik_write
};

static struct proc_dir_entry* own_proc_dir;		// <---. родительская директория
static struct proc_dir_entry* own_proc_node;	// <---. наш файл

static int __init practik_init( void ){
	int ret = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION( 3,10,0 )	// создание файлов проц отличаются в версиях до 3.10.0
	own_proc_dir = create_proc_entry( NAME_DIR, S_IFDIR | S_IRWXUGO, NULL );
	if( own_proc_dir == NULL ){
		ret = -ENOENT;
		ERR( "can't create directory /proc/%s\n", NAME_DIR );
		goto err_dir;
	}
	own_proc_dir->uid = own_proc_dir->gid = 0;
	own_proc_node = create_proc_entry( NAME_NODE, S_IFREG | S_IRUGO | S_IWUGO, own_proc_dir );
	if( own_proc_node == NULL ){
		ret = -ENOENT;
		ERR( "can't create node /proc/%s/%s\n", NAME_DIR, NAME_NODE );
		goto err_node;
	}

	own_proc_node->uid = own_proc_node->gid = 0;
	own_proc_node->proc_fops = &fops;
#else
	own_proc_dir = proc_mkdir( NAME_DIR, NULL );
	if( own_proc_dir == NULL ){
		ret = -ENOENT;
		ERR( "can't create directory /proc/%s\n", NAME_DIR );
		goto err_dir;
	}
	own_proc_node = proc_create( NAME_NODE, S_IFREG | S_IRUGO | S_IWUGO, own_proc_dir, &fops );
	if( own_proc_node == NULL ){
		ret = -ENOENT;
		ERR( "can't create node /proc/%s/%s\n", NAME_DIR, NAME_NODE );
		goto err_node;
	}
#endif

	LOG( "/proc/%s/%s installed\n", NAME_DIR, NAME_NODE );
	return 0;

err_node:
	remove_proc_entry( NAME_DIR, NULL );

err_dir:
	return ret;
}

static void __exit practik_exit( void ){
	remove_proc_entry( NAME_DIR, NULL );
	remove_proc_entry( NAME_NODE, own_proc_dir );
	LOG( "/proc/%s/%s removed\n", NAME_DIR, NAME_NODE );
}

module_init( practik_init );
module_exit( practik_exit );