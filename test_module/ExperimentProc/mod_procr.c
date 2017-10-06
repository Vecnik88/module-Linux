#include "mod_proc.h"
#include "proc_node_read.c"

static int __init proc_init( void ){
	int ret;
	struct proc_dir_entry* own_proc_node;
	own_proc_node = create_proc_entry( NAME_NODE, S_IFREG | S_IRUGO | S_IWUGO, NULL );
}