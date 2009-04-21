#ifndef __MODULE_H
#define __MODULE_H

struct kernel_symbol
{
	long symbol;
	char * name;
};

#define __EXPORT_SYMBOL(sym, sec) \
	static const char __kstrtab_##sym[] __attribute__((section(".ksymtab_strings"))) = "";\
	static const struct kernel_symbol __ksymtab_##sym __attribute__((section(".ksymtab" sec), unused)) = { (long)(&sym), (char *)__kstrtab_##sym }

#define EXPORT_SYMBOL(sym) \
	__EXPORT_SYMBOL(sym, "")

#endif
