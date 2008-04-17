/*
	Ace Kernel Linker Script
*/

kernel_physical_load_address = 0x100000;
kernel_virtual_address = 0xC0000000 + kernel_physical_load_address;

ENTRY (KernelEntry)

SECTIONS
{
	.boot kernel_virtual_address : AT (kernel_physical_load_address)
	{	
		*(.boot*) 
	}
	.text : 
	{   
		*(.text) 	
	}  
	.rodata         :
	{ 
		*(.rodata)
		*(.rodata1)
	}
	. = ALIGN(4096);	/* align to page boundary */
    .data :
	{
		data = .;
		*(.data)
		*(.data1)
		. = ALIGN(4096); 
	}
	.bss : 
	{   sbss = .;
	
		/*kernel page directory*/
		. = ALIGN(4096);
		kernel_page_directory = .;
		. += 4096;
		
		bss = .;
		*(.bss)
		*(COMMON)
		*(.dynbss)
		ebss = .;
	}
	. = ALIGN(4096);
	
	/* Stabs debugging sections.  */
	.stab          0 : { *(.stab) }
	.stabstr       0 : { *(.stabstr) }
	.stab.excl     0 : { *(.stab.excl) }
	.stab.exclstr  0 : { *(.stab.exclstr) }
	.stab.index    0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment       0 : { *(.comment) }

	/* DWARF debug sections.
	Symbols in the DWARF debugging sections are relative to the beginning
	of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
}
