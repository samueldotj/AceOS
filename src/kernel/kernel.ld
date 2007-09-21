
SEARCH_DIR("/usr/lib/gcc/i686-pc-cygwin/3.4.4/")
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



