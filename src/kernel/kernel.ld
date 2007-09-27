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