#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xa2f7d132, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xe70f910f, __VMLINUX_SYMBOL_STR(param_ops_charp) },
	{ 0xa5f266e1, __VMLINUX_SYMBOL_STR(unregister_netdev) },
	{ 0x67f3176f, __VMLINUX_SYMBOL_STR(register_netdev) },
	{ 0x56604461, __VMLINUX_SYMBOL_STR(free_netdev) },
	{ 0x5cef29d0, __VMLINUX_SYMBOL_STR(dev_alloc_name) },
	{ 0xc2b68e70, __VMLINUX_SYMBOL_STR(alloc_netdev_mqs) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x2023de18, __VMLINUX_SYMBOL_STR(ether_setup) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "AFBB4021057054DC33094DF");
