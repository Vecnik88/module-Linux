# Добавляем модуль к списку модулей для компилирования
# module_test.o – означает, что модуль нужно собрать автоматически из module_test.с

obj-m += slave.o

# Подключаем Makefile из kernel-headers с помошью -С,
# который лежит в каталоге /lib/modules/[версия ядра]/build
# используем из него цель modules для сборки модулей в списке obj-m
# M=$(PWD) передаем текущий каталог, где лежат исходники нашего модуля
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean