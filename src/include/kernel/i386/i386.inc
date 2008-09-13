;assembly include file for all i386 kernel assembly files
;contains macros and extern definitions
;note this file should updated as the C header files changes
;todo - make a utility to generate this file from c header files.

MULTIBOOT_PAGE_ALIGN    equ 	1<<0
MULTIBOOT_MEMORY_INFO   equ 	1<<1
MULTIBOOT_AOUT_KLUDGE   equ 	1<<16

MULTIBOOT_HEADER_MAGIC  equ 	0x1BADB002
MULTIBOOT_HEADER_FLAGS  equ 	MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
CHECKSUM                equ 	-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

KERNEL_CODE_SELECTOR	equ		0x8
KERNEL_DATA_SELECTOR	equ		0x10
GDT_ENTRIES			equ		0x5
IDT_ENTRIES 			equ		256

KERNEL_PHYSICAL_ADDRESS	equ 	0x100000
KERNEL_VIRTUAL_ADDRESS	equ 	(0xC0000000 + KERNEL_PHYSICAL_ADDRESS)

CR4_PAGE_SIZE_EXT			equ 	16
CR4_PAGE_GLOBAL_ENABLE	equ		128

%define KERNEL_BOOT_ADDRESS(va)	(va- KERNEL_VIRTUAL_ADDRESS + KERNEL_PHYSICAL_ADDRESS)

KSTACK_SIZE             equ 	0x2000

EXTERN sbss
EXTERN ebss
EXTERN kernel_page_directory
EXTERN gdt
EXTERN idt

EXTERN cmain
EXTERN LoadGdt
EXTERN LoadIdt
EXTERN InitPhysicalMemoryManagerPhaseI
EXTERN SecondaryCpuStart
