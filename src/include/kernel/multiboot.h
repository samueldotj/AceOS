/*! \file 	multiboot.h
	\brief 	The header for Multiboot
*/
#ifndef __MULTIBOOT__H
#define __MULTIBOOT__H

/*! The magic number for the Multiboot header.  */
#define MULTIBOOT_HEADER_MAGIC			0x1BADB002

/*! The flags for the Multiboot header.  */
#ifdef __ELF__
	#define MULTIBOOT_HEADER_FLAGS		0x00000003
#else
	#define MULTIBOOT_HEADER_FLAGS		0x00010003
#endif

/*! The magic number passed by a Multiboot-compliant boot loader.  */
#define MULTIBOOT_BOOTLOADER_MAGIC		0x2BADB002

/*! The size of our stack (16KB).  */
#define STACK_SIZE						0x4000

/*! C symbol format. HAVE_ASM_USCORE is defined by configure.  */
#ifdef HAVE_ASM_USCORE
	#define EXT_C(sym)			_ ## sym
#else
	#define EXT_C(sym)			sym
#endif


#define MB_FLAG_MEM			1
#define MB_FLAG_BOOTDEVICE	2
#define MB_FLAG_CMD			4
#define MB_FLAG_MODS		8
#define MB_FLAG_AOUT		16
#define MB_FLAG_ELF			32
#define MB_FLAG_MMAP		64
#define MB_FLAG_DRIVE		128
#define MB_FLAG_ROM			256
#define MB_FLAG_BOOTLOADER	512
#define MB_FLAG_APM			1024
#define MB_FLAG_GRAPHICS	2048

#ifndef ASM

/*! The Multiboot header.  */
typedef struct multiboot_header
{
	unsigned long magic;
	unsigned long flags;
	unsigned long checksum;
	unsigned long header_addr;
	unsigned long load_addr;
	unsigned long load_end_addr;
	unsigned long bss_end_addr;
	unsigned long entry_addr;
}MULTIBOOT_HEADER, *MULTIBOOT_HEADER_PTR;

/*! The symbol table for a.out.  */
typedef struct aout_symbol_table
{
	unsigned long tabsize;
	unsigned long strsize;
	unsigned long addr;
	unsigned long reserved;
}MULTIBOOT_AOUT_SYMBOL_TABLE, *MULTIBOOT_AOUT_SYMBOL_TABLE_PTR;

/*! The section header table for ELF.  */
typedef struct elf_section_header_table
{
	unsigned long num;
	unsigned long size;
	unsigned long addr;
	unsigned long shndx;
}MULTIBOOT_ELF_SECTION_HEADER_TABLE, *MULTIBOOT_ELF_SECTION_HEADER_TABLE_PTR;

/*! The Multiboot information.  */
typedef struct multiboot_info
{
	unsigned long flags;
	unsigned long mem_lower;
	unsigned long mem_upper;
	unsigned long boot_device;
	unsigned long cmdline;
	unsigned long mods_count;
	unsigned long mods_addr;
	union
	{
		MULTIBOOT_AOUT_SYMBOL_TABLE 		aout_sym;
		MULTIBOOT_ELF_SECTION_HEADER_TABLE 	elf_sec;
	};
	unsigned long mmap_length;
	unsigned long mmap_addr;
}MULTIBOOT_INFO, * MULTIBOOT_INFO_PTR;

/*! The module structure.  */
typedef struct module
{
	unsigned long mod_start;
	unsigned long mod_end;
	unsigned long string;
	unsigned long reserved;
}MULTIBOOT_MODULE, *MULTIBOOT_MODULE_PTR;

/*! The memory map. Be careful that the offset 0 is base_addr_low but no size.  */
typedef struct memory_map
{
  unsigned long size;
  unsigned long base_addr_low;
  unsigned long base_addr_high;
  unsigned long length_low;
  unsigned long length_high;
  unsigned long type;
}MULTIBOOT_MEMORY_MAP, * MULTIBOOT_MEMORY_MAP_PTR;

#endif /* ! ASM */

#endif
