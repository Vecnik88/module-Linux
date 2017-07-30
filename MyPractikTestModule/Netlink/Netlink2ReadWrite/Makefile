TARGET = netlink

obj-m := $(TARGET).o

all: user

user: user.c
	gcc user.c -o user

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
