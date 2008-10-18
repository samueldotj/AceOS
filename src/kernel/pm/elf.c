#include <kernel/error.h>
#include <kernel/pm/elf.h>
#include <kernel/mm/vm_types.h>

ERROR_CODE LoadElfImage(VADDR elf_image, VIRTUAL_MAP_PTR virtual_map)
{
	ELF32_HEADER_PTR elf_header = (ELF32_HEADER_PTR)elf_image;
	if ( elf_header->e_ident[EI_MAGIC0] != ELFMAGIC0 || elf_header->e_ident[EI_MAGIC1] != ELFMAGIC1 || 
		elf_header->e_ident[EI_MAGIC2] != ELFMAGIC2 || elf_header->e_ident[EI_MAGIC3] != ELFMAGIC3 )
		return ERROR_NOT_SUPPORTED;
	kprintf("ELF file");
	
	return ERROR_SUCCESS;
}
