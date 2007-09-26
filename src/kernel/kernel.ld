SEARCH_DIR("/usr/lib/gcc/i686-pc-cygwin/3.4.4/")
SEARCH_DIR("/usr/lib/gcc/i386-redhat-linux/4.1.1/")
kernel_virtual_address =  0x100000;
ENTRY (_KernelEntry)
SECTIONS
{
	.boot kernel_virtual_address :
	{
		*(.boot*)
	}
	.text :
    {   
		*(.text)
		*(.rodata*)         /* ELF read-only data section(s) */
		. = ALIGN(4096);	/* align to page boundary */
	}  
    .data :
	{
		data = .;
		*(.data)
		. = ALIGN(4096); 
	}
	.bss : 
	{   _sbss = .;
		bss = .;
		*(.bss)
		*(COMMON)
		. = ALIGN(4096);
		_ebss = .;
	}
}