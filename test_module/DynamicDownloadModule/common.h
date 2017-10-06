#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <fcntl.h>

#define SLAVE_FILE "./slave.ko"
#define BUF_MES 80
