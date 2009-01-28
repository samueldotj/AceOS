#include <stdlib.h>
#include <string.h>
#include <kernel/error.h>
#include <kernel/debug.h>
#include <kernel/pm/task.h>
#include <kernel/pm/elf.h>

static ERROR_CODE LoadElfSections(ELF_HEADER_PTR file_header, char * string_table, VIRTUAL_MAP_PTR virtual_map, char * start_symbol_name, VADDR * start_entry);
static ERROR_CODE LoadElfSectionIntoMap(ELF_SECTION_HEADER_PTR section_header, VADDR section_data_offset, VIRTUAL_MAP_PTR virtual_map, VADDR * section_loaded_va);
static ERROR_CODE RelocateSection(ELF_SECTION_HEADER_PTR section_header, ELF_SYMBOL_PTR symbol_table, int relocation_section_index, char * string_table, VADDR relocation_entries, VADDR * section_loaded_va);
static ELF_SYMBOL_PTR FindElfSymbolByName(ELF_SYMBOL_PTR symbol_table, UINT32 symbol_table_size, char * string_table, char * symbol_name);

#if ARCH == i386
static void RelocateI386Field(ELF32_RELOCATION_PTR rp, VADDR symbol_value, VADDR base_va, int type);
#endif

/*! Loads an ELF image into the given virtual map
	\param file_header - virtual address where the elf file resides
	\param virtual_map - the elf sections will be loaded into this virtual map
	\param start_symbol_name - program's first function to start
	\param start_entry - if start_symbol_name is not null then the symbol will be searched in the symbol table and if found its loaded VA will be updated here.
	
	This function is just a wrapper LoadElfSections(). It just performs some sanity checks and give calls LoadElfSections()
*/
ERROR_CODE LoadElfImage(ELF_HEADER_PTR file_header, VIRTUAL_MAP_PTR virtual_map, char * start_symbol_name, VADDR * start_entry)
{
	ELF_SECTION_HEADER_PTR string_section_header=NULL;
	char * string_table = NULL;
	
	/*check for correct file type and machine type*/
	if( file_header->e_ident[EI_MAGIC0] != ELFMAGIC0 || file_header->e_ident[EI_MAGIC1] != ELFMAGIC1 || 
		file_header->e_ident[EI_MAGIC2] != ELFMAGIC2 || file_header->e_ident[EI_MAGIC3] != ELFMAGIC3 )
		return ERROR_INVALID_FORMAT;
#if ARCH == i386
	if( file_header->e_ident[EI_CLASS] != ELFCLASS32 || file_header->e_ident[EI_DATA] != ELFDATA2LSB )
		return ERROR_INVALID_FORMAT;
#endif
	if ( file_header->e_type == ET_NONE )
		return ERROR_NOT_SUPPORTED;

	/*loop through the segments and load the segments*/
	if ( file_header->e_shoff != 0 )
	{
		/*load the string table*/
		if ( file_header->e_shstrndx != SHN_UNDEF )
		{
			string_section_header = (ELF_SECTION_HEADER_PTR)( (VADDR)file_header + file_header->e_shoff + (file_header->e_shstrndx*file_header->e_shentsize) );
			if ( string_section_header->sh_type != SHT_STRTAB )
				return ERROR_NOT_SUPPORTED;
			string_table = (char *)file_header + string_section_header->sh_offset;
		}
		/*load the elf section and relocate it if needed*/
		LoadElfSections(file_header, string_table, virtual_map, start_symbol_name, start_entry);
	}
	
	return ERROR_SUCCESS;
}
/*! Loads ELF sections into given virtual map and fixes the relocations
	\param file_header - virtual address where the elf file resides
	\param string_table - string table for the elf section names
	\param virtual_map - the elf sections will be loaded into this virtual map
	\param start_symbol_name - program's first function to start
	\param start_entry - if start_symbol_name is not null then the symbol will be searched in the symbol table and if found its loaded VA will be updated here.
*/
static ERROR_CODE LoadElfSections(ELF_HEADER_PTR file_header, char * string_table, VIRTUAL_MAP_PTR virtual_map, char * start_symbol_name, VADDR * start_entry)
{
	ELF_SECTION_HEADER_PTR start_section_header, section_header;
	VADDR * section_loaded_va = NULL;
	ERROR_CODE ret = ERROR_SUCCESS;
	int i;
	
	/*if start symbol needs to be returned initialize the value*/
	if( start_symbol_name )
	{
		assert( start_entry != NULL );
		*start_entry = NULL;
	}	
	
	/*section_loaded_va holds address of where the section is loaded into the virtual map*/
	section_loaded_va  = (VADDR *) kmalloc( sizeof(VADDR) * file_header->e_shnum, 0 );
	if ( section_loaded_va  == NULL )
		return ERROR_NOT_ENOUGH_MEMORY;

	/*start of the section header*/
	start_section_header = (ELF_SECTION_HEADER_PTR)( (VADDR)file_header + file_header->e_shoff );
	
	/*first pass - load all the sections into address space*/
	for(i=0, section_header = start_section_header; i<file_header->e_shnum;i++, section_header = (ELF_SECTION_HEADER_PTR) (((VADDR)section_header) + file_header->e_shentsize))
	{
		/*analyze only active sections*/
		if ( section_header->sh_type == SHT_NULL )
			continue;
		
		ret = LoadElfSectionIntoMap(section_header, (VADDR)file_header+section_header->sh_offset, virtual_map, &section_loaded_va[i]);
		if ( ret != ERROR_SUCCESS )
			goto done;
	}
	/*second pass - relocate and find start symbol*/
	for(i=0, section_header = start_section_header; i<file_header->e_shnum;i++, section_header = (ELF_SECTION_HEADER_PTR) (((VADDR)section_header) + file_header->e_shentsize))
	{
		/*relocate the image if needed*/
		if ( section_header->sh_type == SHT_RELA || section_header->sh_type == SHT_REL )
		{
			int symbol_table_index, relocation_section_index;
			ELF_SECTION_HEADER_PTR symbol_table_header, string_table_header;
			
			symbol_table_index = section_header->sh_link;
			relocation_section_index = section_header->sh_info;

			symbol_table_header = &start_section_header[section_header->sh_link];
			string_table_header = &start_section_header[symbol_table_header->sh_link];
			ret = RelocateSection( section_header, (ELF_SYMBOL_PTR) ((char*)file_header + symbol_table_header->sh_offset),
				relocation_section_index, (char*)file_header + string_table_header->sh_offset, (VADDR)file_header + section_header->sh_offset, section_loaded_va );
			if ( ret != ERROR_SUCCESS )
				goto done;
		}
		/*find start symbol - if start symbol name is given and start entry is not yet found*/
		if ( start_symbol_name && *start_entry==NULL && ( section_header->sh_type == SHT_SYMTAB || section_header->sh_type == SHT_DYNSYM ) )
		{
			char * string_table = (char *)file_header + start_section_header[section_header->sh_link].sh_offset;
			ELF_SYMBOL_PTR sym = FindElfSymbolByName( (ELF_SYMBOL_PTR)((char*)file_header + section_header->sh_offset), section_header->sh_size , string_table, start_symbol_name);
			if ( sym != NULL )
				*start_entry = section_loaded_va[sym->st_shndx] + sym->st_value;
		}
	}

done:
	kfree( section_loaded_va );
	return ret;
}
/*! Loads a given ELF section into a Virtual Map and returns the loaded virtual address in section_loaded_va
		Sections are loaded only if the section has SHF_ALLOC otherwise just ERROR_SUCCESS will returned without loading and section_loaded_va will set to NULL
	\param section_header - section header
	\param section_data_offset - virtual address where the section data resides in the kernel/current map
	\param virtual_map - the target process's virtual map where this section should be loaded
	\param section_loaded_va - pointer to va - where the loaded VA will be updated
*/
static ERROR_CODE LoadElfSectionIntoMap(ELF_SECTION_HEADER_PTR section_header, VADDR section_data_offset, VIRTUAL_MAP_PTR virtual_map, VADDR * section_loaded_va)
{
	UINT32 protection = PROT_READ;
	ERROR_CODE ret = ERROR_SUCCESS;
	
	/*be pessimistic*/
	* section_loaded_va = NULL;
	
	/*allocate virtual address space if the section occupies space during exeuction*/
	if( !(section_header->sh_flags & SHF_ALLOC) )
		return ret;
			
	/*update protection*/
	if (section_header->sh_flags & SHF_WRITE)
		protection |= PROT_WRITE;
	if (section_header->sh_flags & SHF_EXECINSTR)
		protection |= PROT_EXECUTE;
	
	/*hint the VM our preferred virtual address*/
	*section_loaded_va = section_header->sh_addr;
	/*copy section data only if the source section has some data to copy*/
	if ( !(section_header->sh_flags & SHT_NOBITS) && (section_header->sh_size > 0) )
	{
		ret = CopyVirtualAddressRange( GetCurrentVirtualMap(), section_data_offset, virtual_map, section_loaded_va, section_header->sh_size, protection);
	}
	else
	{
		/*else create a new zero filled section*/
		VADDR size = section_header->sh_size;
		if ( size == 0 )
			size = PAGE_SIZE;
		ret = AllocateVirtualMemory( virtual_map, section_loaded_va, section_header->sh_addr, size, protection, 0, NULL );
	}
	/*adjust the section loaded address*/
	*section_loaded_va += (section_data_offset - PAGE_ALIGN(section_data_offset) );
	
	return ret;
}

/*! Fixes the relocation entries by walking all the relocation entries and calling architecture specific fixup routines*/
static ERROR_CODE RelocateSection(ELF_SECTION_HEADER_PTR section_header, ELF_SYMBOL_PTR symbol_table, int relocation_section_index, char * string_table, VADDR relocation_entries, VADDR * section_loaded_va)
{
	int i, total_entries;
	
	total_entries =  section_header->sh_size / ( section_header->sh_type == SHT_REL ? sizeof(ELF_RELOCATION) : sizeof(ELF_RELOCATION_ADDEND) );
	for( i=0; i< total_entries; i++)
	{
		int sym_index, type;	
		if ( section_header->sh_type == SHT_RELA )
		{
			ELF_RELOCATION_ADDEND_PTR rpa = &((ELF_RELOCATION_ADDEND_PTR)relocation_entries)[i];
			sym_index = ELF_R_SYM(rpa->r_info);
			type = ELF_R_TYPE(rpa->r_info);
#if ARCH == i386
			/*i386 doesnt support relocation_addend format*/
			return ERROR_INVALID_FORMAT;
#endif
		}
		else
		{
			ELF_RELOCATION_PTR rp = &((ELF_RELOCATION_PTR)relocation_entries)[i];
			VADDR symbol_value = 0;
			sym_index = ELF_R_SYM(rp->r_info);
			type = ELF_R_TYPE(rp->r_info);
			/*if the symbol points to undefined symbol table then we have to load the symbol value from dynamic library*/
			if ( symbol_table[sym_index].st_shndx == SHN_UNDEF )
			{
				ELF_SYMBOL_PTR symbol;
				/*\todo - load the symbol value from the dynamic library*/
				
				/*search kernel symbols and update the symbol value.*/
				symbol = FindElfSymbolByName( (ELF_SYMBOL_PTR)kernel_reserve_range.symbol_va_start, kernel_reserve_range.symbol_va_end-kernel_reserve_range.symbol_va_start, 
												(char *)kernel_reserve_range.string_va_start, &string_table[ symbol_table[sym_index].st_name ]  );
				if ( symbol  )
					symbol_value = symbol->st_value;
				else
					return ERROR_SYMBOL_NOT_FOUND;
			}
			else
			{
				symbol_value = section_loaded_va[symbol_table[sym_index].st_shndx] + symbol_table[sym_index].st_value;
			}
#if ARCH == i386
			if ( section_loaded_va[relocation_section_index] != NULL )
				RelocateI386Field(rp, symbol_value, section_loaded_va[relocation_section_index], type);
#endif
		}
	}
	return ERROR_SUCCESS;
}
/*! Searches for a given symbol name in the given symbol table
	\param symbol_table - symbol table to search
	\param symbol_table_size - size of the symbol table in bytes
	\param string_table - string table associated with the symbol table
	\param symbol_name - symbol name to search
	\return on success - SYMBOL address 
			on failure - NULL
*/
static ELF_SYMBOL_PTR FindElfSymbolByName(ELF_SYMBOL_PTR symbol_table, UINT32 symbol_table_size, char * string_table, char * symbol_name)
{
	int i;
	for(i=0;i<symbol_table_size/sizeof(ELF_SYMBOL) ;i++)
	{
		if ( strcmp( string_table+symbol_table[i].st_name, symbol_name ) == 0 )
				return &symbol_table[i];
	}
	kprintf("ELF: Symbol not found %s(%p:%d)\n", symbol_name, symbol_table, symbol_table_size/sizeof(ELF_SYMBOL));
	return NULL;
}

#if ARCH == i386
/*! i386 specific relocation fixup code
	\param rp - relocation entry
	\param symbol_value - symbol value
	\param base_va - virtual address where the section associated with the relocation is loaded
	\param type - relocation type
*/
static void RelocateI386Field(ELF32_RELOCATION_PTR rp, VADDR symbol_value, VADDR base_va, int type)
{
	UINT32 S, A, P;
	UINT32 * relocate_offset = (UINT32 *)(base_va + rp->r_offset);
	
	P = (UINT32)relocate_offset;
	S = symbol_value;
	/*i386 doesnt uses the Addend field - Addend is taken from the field to be relocated*/
	A = *relocate_offset;
	switch( type )
	{
		case R_386_NONE:
		case R_386_COPY:
			break;
		case R_386_32:
			*relocate_offset = S + A;
			break;
		case R_386_PC32:
			*relocate_offset = S + A - P;
			break;
		case R_386_GLOB_DAT:
		case R_386_JMP_SLOT:
			*relocate_offset = S;
			break;
		case R_386_GOT32:
		case R_386_PLT32:
		case R_386_RELATIVE:
		case R_386_GOTPC:
		case R_386_GOTOFF:
			panic("ELF - Unsupported relocation entry type");
			break;
		default:
			panic("ELF - Unknown relocation entry type");
	}
}
#endif
