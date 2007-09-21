/*
	Ace Kernel Linker Script - Copied from Mobius OS
*/
OUTPUT_FORMAT("elf32-i386")
SEARCH_DIR("/lib")
SEARCH_DIR("/usr/lib/gcc/i386-redhat-linux/4.1.1/")

kernel_virtual_address =  0x100000;
ENTRY (_KernelEntry)
SECTIONS
{
	.text  kernel_virtual_address :
	{
		*(.text)
	}
	.bss :
    {
		__bss_start__ = . ;
        _sbss = .;
        *(.bss)
        *(COMMON)

        . = ALIGN(4096);
        _ebss = .;
        __bss_end__ = . ;

	}
}



