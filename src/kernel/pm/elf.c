#include <stdlib.h>
#include <string.h>
#include <ds/bits.h>
#include <kernel/error.h>
#include <kernel/debug.h>
#include <kernel/pm/task.h>
#include <kernel/pm/elf.h>

static ERROR_CODE LoadElfSegments(ELF_HEADER_PTR file_header, char * string_table, VIRTUAL_MAP_PTR virtual_map);
static ERROR_CODE LoadElfSegmentIntoMap(ELF_PROGRAM_HEADER_PTR program_header, char *segment_start, VIRTUAL_MAP_PTR virtual_map);
static ERROR_CODE LoadElfSections(ELF_HEADER_PTR file_header, char * string_table, VIRTUAL_MAP_PTR virtual_map, VADDR * section_loaded_va);
static ERROR_CODE LoadElfSectionIntoMap(ELF_SECTION_HEADER_PTR section_header, VADDR section_data_offset, VIRTUAL_MAP_PTR virtual_map, VADDR * section_loaded_va);
static ERROR_CODE RelocateSection(ELF_SECTION_HEADER_PTR section_header, ELF_SYMBOL_PTR symbol_table, int relocation_section_index, char * string_table, VADDR relocation_entries, VADDR * section_loaded_va);
static VADDR FindSymbolAddress(ELF_HEADER_PTR file_header, char * symbol_name, VADDR * section_loaded_va);
static ELF_SYMBOL_PTR FindElfSymbolByName(ELF_SYMBOL_PTR symbol_table, UINT32 symbol_table_size, char * string_table, char * symbol_name);
static UINT32 GetElfCommonSectionSize(ELF_SYMBOL_PTR symbol_table, UINT32 symbol_table_size);
static void UpdateElfCommonSectionSymbols(ELF_SYMBOL_PTR symbol_table, UINT32 symbol_table_size, int common_section_index);

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
	ELF_SECTION_HEADER_PTR string_section_header = NULL;
	char * string_table = NULL;
	VADDR * section_loaded_va = NULL;			/*holds array of virtual addresses, where the sections are loaded into the virtual map*/
	ERROR_CODE err = ERROR_NOT_SUPPORTED;
	int i;
	
	assert(file_header != NULL);
	
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
		
	/*  Allocate memory to hold the virtual addresses where sections will be loaded.
		The last entry is used for common section(SHN_COMMON).
	*/
	section_loaded_va  = (VADDR *) kmalloc( sizeof(VADDR) * file_header->e_shnum+1, 0 );
	if ( section_loaded_va  == NULL )
		return ERROR_NOT_ENOUGH_MEMORY;
	for(i=0; i<=file_header->e_shnum; i++)
		section_loaded_va[i] = NULL;

	/*find string table*/
	if ( file_header->e_shoff != 0 && file_header->e_shstrndx != SHN_UNDEF )
	{
		string_section_header = (ELF_SECTION_HEADER_PTR)( (VADDR)file_header + file_header->e_shoff + (file_header->e_shstrndx*file_header->e_shentsize) );
		/*just make sure we have correct section type*/
		if ( string_section_header->sh_type != SHT_STRTAB )
			return ERROR_INVALID_FORMAT;
		
		string_table = (char *)file_header + string_section_header->sh_offset;
	}

	/*if program header present load all segments*/
	if (  file_header->e_phoff != 0 )
	{
		/*load the elf section and relocate it */
		err = LoadElfSegments(file_header, string_table, virtual_map);
	}
	/*else load all segments*/
	else if ( file_header->e_shoff != 0 )
	{
		/*load the elf section and relocate it */
		err = LoadElfSections(file_header, string_table, virtual_map, section_loaded_va);
	}
	
	assert( start_entry != NULL );
	*start_entry = NULL;
	
	/*if start symbol name given find its address*/
	if ( start_symbol_name )
	{
		*start_entry = FindSymbolAddress( file_header, start_symbol_name, section_loaded_va);
	}
	/*get the start entry from elf header*/
	else 
	{
		*start_entry =  file_header->e_entry;
	}
	
	kfree( section_loaded_va );
	return err;
}
static ERROR_CODE LoadElfSegments(ELF_HEADER_PTR file_header, char * string_table, VIRTUAL_MAP_PTR virtual_map)
{
	ELF_PROGRAM_HEADER_PTR first_program_header, program_header;
	ERROR_CODE ret = ERROR_SUCCESS;
	int i;
	
	/*start of the section header*/
	first_program_header = (ELF_PROGRAM_HEADER_PTR)( (VADDR)file_header + file_header->e_phoff );
	
	/*load all the segments into address space*/
	for(i=0, program_header = first_program_header; i<file_header->e_phnum;i++, program_header = (ELF_PROGRAM_HEADER_PTR) (((VADDR)program_header) + file_header->e_phentsize))
	{
		if ( program_header->p_type == PT_NULL )
		{
			continue;
		}
		else
		{
			ret = LoadElfSegmentIntoMap(program_header, ((char *)file_header)+program_header->p_offset, virtual_map );
			if ( ret != ERROR_SUCCESS )
				return ret;
		}
	}
	
	return ret;
}
static ERROR_CODE LoadElfSegmentIntoMap(ELF_PROGRAM_HEADER_PTR program_header, char * segment_start, VIRTUAL_MAP_PTR virtual_map)
{
	ERROR_CODE ret = ERROR_SUCCESS;
	UINT32 protection = 0;
	VADDR preferred_va;
	
	/*load only segments which occupies memory others skip silently*/
	if ( program_header->p_memsz == 0 )
		return ret;
	
	/*get the correct permission for the segment*/
	if ( program_header->p_flags & PF_R )
		protection |= PROT_READ;
	if ( program_header->p_flags & PF_W )
		protection |= PROT_WRITE;
	if ( program_header->p_flags & PF_X )
		protection |= PROT_EXECUTE;

	preferred_va = program_header->p_vaddr;

	/*if the segment is loadable - map it into the address space*/
	if ( program_header->p_type == PT_LOAD )
	{
		ret = CopyVirtualAddressRange( GetCurrentVirtualMap(), (VADDR)segment_start, program_header->p_filesz, virtual_map, &preferred_va, program_header->p_memsz, protection );
	}
	else
	{
		/*else create a new zero filled section*/
		typeof(program_header->p_memsz) size = program_header->p_memsz;
		if ( size == 0 )
			size = PAGE_SIZE;
		/* \todo - ensure we load into preferred address else do the relocation...*/
		ret = AllocateVirtualMemory( virtual_map, &preferred_va, program_header->p_vaddr, size, protection, 0, NULL );
	}
	
	return ret;
}
/*! Loads ELF sections into given virtual map and fixes the relocations
	\param file_header - virtual address where the elf file resides
	\param string_table - string table for the elf section names
	\param virtual_map - the elf sections will be loaded into this virtual map
	\param section_loaded_va - pointer to section's va 
*/
static ERROR_CODE LoadElfSections(ELF_HEADER_PTR file_header, char * string_table, VIRTUAL_MAP_PTR virtual_map, VADDR * section_loaded_va)
{
	ELF_SECTION_HEADER_PTR first_section_header, section_header;
	ELF_SYMBOL_PTR symbol_table=NULL;
	UINT32 symbol_table_size=0;
	ERROR_CODE ret = ERROR_SUCCESS;
	int i;
	UINT32 common_size = 0;
	
	/*start of the section header*/
	first_section_header = (ELF_SECTION_HEADER_PTR)( (VADDR)file_header + file_header->e_shoff );
	
	/*first pass - 
		1) load all the sections into address space
		2) find common section address space
	*/
	for(i=0, section_header = first_section_header; i<file_header->e_shnum;i++, section_header = (ELF_SECTION_HEADER_PTR) (((VADDR)section_header) + file_header->e_shentsize))
	{
		/*analyze only active sections*/
		if ( section_header->sh_type == SHT_NULL )
		{
			section_loaded_va[i] = NULL;
			continue;
		}
		/*symbol table section - get the common section size*/
		if ( section_header->sh_type == SHT_SYMTAB )
		{
			symbol_table = (ELF_SYMBOL_PTR)((char*)file_header + section_header->sh_offset);
			symbol_table_size = section_header->sh_size;
			common_size += GetElfCommonSectionSize( symbol_table, symbol_table_size );
		}
		ret = LoadElfSectionIntoMap(section_header, (VADDR)file_header+section_header->sh_offset, virtual_map, &section_loaded_va[i]);
		if ( ret != ERROR_SUCCESS )
			return ret;
	}
	/*if common section is present, allocate memory for it and update all common symbols*/
	if( common_size != 0 )
	{
		/*allocate memory*/
		ret = AllocateVirtualMemory( virtual_map, &section_loaded_va[file_header->e_shnum], 0, PAGE_ALIGN_UP(common_size), PROT_READ | PROT_WRITE, 0, NULL );
		if ( ret != ERROR_SUCCESS )
			return ret;
		/*Update the symbols*/
		UpdateElfCommonSectionSymbols(symbol_table, symbol_table_size, file_header->e_shnum);
	}
	
	/*second pass - relocate */
	for(i=0, section_header = first_section_header; i<file_header->e_shnum;i++, section_header = (ELF_SECTION_HEADER_PTR) (((VADDR)section_header) + file_header->e_shentsize))
	{
		/*relocate the image if needed*/
		if ( section_header->sh_type == SHT_RELA || section_header->sh_type == SHT_REL )
		{
			int symbol_table_index, relocation_section_index;
			ELF_SECTION_HEADER_PTR symbol_table_header, string_table_header;
			
			symbol_table_index = section_header->sh_link;
			relocation_section_index = section_header->sh_info;

			symbol_table_header = &first_section_header[section_header->sh_link];
			string_table_header = &first_section_header[symbol_table_header->sh_link];
			ret = RelocateSection( section_header, (ELF_SYMBOL_PTR) ((char*)file_header + symbol_table_header->sh_offset),
				relocation_section_index, (char*)file_header + string_table_header->sh_offset, (VADDR)file_header + section_header->sh_offset, section_loaded_va );
			if ( ret != ERROR_SUCCESS )
				return ret;
		}
	}

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
	
	/*allocate virtual address space if the section occupies space during exeuction
	if( !(section_header->sh_flags & SHF_ALLOC) )
		return ret;*/
			
	/*update protection*/
	if (section_header->sh_flags & SHF_WRITE)
		protection |= PROT_WRITE;
	if (section_header->sh_flags & SHF_EXECINSTR)
		protection |= PROT_EXECUTE;
	
	/*hint the VM our preferred virtual address*/
	*section_loaded_va = section_header->sh_addr;
	
	/*create a new zero filled section if bss*/
	if ( section_header->sh_type == SHT_NOBITS )
	{
		VADDR size = section_header->sh_size;
		if ( size == 0 )
			size = PAGE_SIZE;
		ret = AllocateVirtualMemory( virtual_map, section_loaded_va, section_header->sh_addr, size, protection, 0, NULL );
	}
	else
	{
		/*copy section data only if the source section has some data to copy*/
		ret = CopyVirtualAddressRange( GetCurrentVirtualMap(), section_data_offset, section_header->sh_size, virtual_map, section_loaded_va, section_header->sh_size, protection);
	}
	
	return ret;
}

/*! Searches for a given symbol name in the given symbol table and returns its loaded virtual address
	\param file_header - virtual address where the elf file resides
	\param symbol_name - symbol name to search
	\param section_loaded_va - pointer to array of virtual address where sections are loaded
	\return on success - SYMBOL's virtual address 
			on failure - NULL
*/
static VADDR FindSymbolAddress(ELF_HEADER_PTR file_header, char * symbol_name, VADDR * section_loaded_va)
{
	VADDR result = NULL;
	ELF_SYMBOL_PTR sym;
	int i;
	
	ELF_SECTION_HEADER_PTR first_section_header, section_header;
	
	if ( file_header->e_shoff == 0 )
		return NULL;
	
	first_section_header = (ELF_SECTION_HEADER_PTR)( (VADDR)file_header + file_header->e_shoff );
	/*loop through the section tables to find symbol table*/
	for(i=0, section_header = first_section_header; i<file_header->e_shnum;i++, section_header = (ELF_SECTION_HEADER_PTR) (((VADDR)section_header) + file_header->e_shentsize))
	{
		char * string_table;
		int string_table_index;
		if ( section_header->sh_type != SHT_SYMTAB && section_header->sh_type != SHT_DYNSYM )
			continue;

		/*try to use section specific string table if present else use file specific string table, if both not present return NULL*/
		if(  section_header->sh_link != SHN_UNDEF )
			string_table_index = section_header->sh_link;
		else if ( file_header->e_shstrndx != SHN_UNDEF )
			string_table_index = file_header->e_shstrndx;
		else 
			return NULL;
		string_table = ((char *)file_header) + first_section_header[string_table_index].sh_offset;
		sym = FindElfSymbolByName( (ELF_SYMBOL_PTR)((char*)file_header + section_header->sh_offset), section_header->sh_size, string_table, symbol_name);
		if ( sym == NULL )
			continue;
		/*ok - we found our symbol just get the correct address based on the type*/
		if ( sym->st_shndx == SHN_ABS )
		{
			/*absolute value - ace supports?*/
			result = sym->st_value;
		}
		else if ( sym->st_shndx == SHN_COMMON || sym->st_shndx == SHN_UNDEF )
		{
			KTRACE("sym->st_shndx == SHN_COMMON || sym->st_shndx == SHN_UNDEF");
		}
		else 
		{
			/*If a symbol’s value refers to a speciﬁc location within a section, its section index member, st_shndx, holds an index into the section header table.
			If we relocated that section, then find the correct va*/
			ELF_SECTION_HEADER_PTR other_section_header = (ELF_SECTION_HEADER_PTR) (((VADDR)first_section_header) + (file_header->e_shentsize * sym->st_shndx) );
			VADDR relocated_va = sym->st_value - other_section_header->sh_addr;
			if ( section_loaded_va )
				result = section_loaded_va[sym->st_shndx] + relocated_va;
			else
				result = sym->st_value;
		}
		break;
	}
	
	return result;
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
	KTRACE("Symbol not found %s(%p:%d) string table %p\n", symbol_name, symbol_table, symbol_table_size/sizeof(ELF_SYMBOL), string_table);
	return NULL;
}

/*! Searches for a given symbol name in the given symbol table
	\param address 	- kernel address to look
	\param offset	- offset from the symbol name will be updated here
	\return on success - SYMBOL name 
			on failure - NULL
*/
char * FindKernelSymbolByAddress(VADDR address, int * offset)
{
	int i;
	ELF_SYMBOL_PTR symbol_table = (ELF_SYMBOL_PTR)kernel_reserve_range.symbol_va_start;
	UINT32 total_symbols = (kernel_reserve_range.symbol_va_end-kernel_reserve_range.symbol_va_start)/sizeof(ELF_SYMBOL);
	char * string_table = (char *)kernel_reserve_range.string_va_start;
	
	/*loop through kernel symbol table entries*/
	for(i=0;i<total_symbols;i++)
	{
		/*if the given address lies within the symbol value return symbol name*/
		if ( VALUE_WITH_IN_RANGE(symbol_table[i].st_value, symbol_table[i].st_value+symbol_table[i].st_size, address) )
		{
			/*if the caller wants offset from the symbol start, update it*/
			if ( offset )
				*offset = address-symbol_table[i].st_value;
			
			/*return the symbol name string*/
			return string_table+symbol_table[i].st_name;
		}
	}
	
	/*no matching symbol found*/
	return NULL;
}

/*! Returns common section size by summing up all the symbol's size of type common.
	\param symbol_table - symbol table to search
	\param symbol_table_size - size of the symbol table in bytes
	\return size of the common section
*/
static UINT32 GetElfCommonSectionSize(ELF_SYMBOL_PTR symbol_table, UINT32 symbol_table_size)
{
	UINT32 result = 0;
	int i;
	for(i=0;i<symbol_table_size/sizeof(ELF_SYMBOL) ;i++)
	{
		if ( symbol_table[i].st_shndx == SHN_COMMON )
		{
		    /* In the case of an SHN_COMMON symbol the st_value field holds alignment constraints.*/
            UINT32 boundary;
			boundary = symbol_table[i].st_value - 1;

            /* Calculate the next byte boundary.*/
            result = ( result + boundary ) & ~boundary;
            result += symbol_table[i].st_size;
		}
	}
	return result;
}

/*! Update all symbols of type common section with the following values
		1) index of common section
		2) offset
	\param symbol_table - symbol table to search
	\param symbol_table_size - size of the symbol table in bytes
	\param common_section_index - common section index(currently section count+1)
*/
static void UpdateElfCommonSectionSymbols(ELF_SYMBOL_PTR symbol_table, UINT32 symbol_table_size, int common_section_index)
{
	UINT32 offset = 0;
	int i;
	for(i=0;i<symbol_table_size/sizeof(ELF_SYMBOL) ;i++)
	{
		if ( symbol_table[i].st_shndx == SHN_COMMON )
		{
		    /* In the case of an SHN_COMMON symbol the st_value field holds alignment constraints.*/
            UINT32 boundary;
			boundary = symbol_table[i].st_value - 1;

            /* Calculate the next byte boundary.*/
            offset = ( offset + boundary ) & ~boundary;
			symbol_table[i].st_shndx = common_section_index;
			symbol_table[i].st_value = offset;
            offset += symbol_table[i].st_size;
		}
	}
}


/*! Fixes the relocation entries by walking all the relocation entries and calling architecture specific fixup routines
	\param section_header - Relocation section header
	\param symbol_table - Symbol table associated with the section
	\param relocation_section_index - Section to which relocation to be applied
	\param string_table - String table associated with the section
	\param relocation_entries - array of relocation entries.
	\param section_loaded_va - array of virtual addresses where all the sections are loaded
*/
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
			KTRACE("section_header->sh_type == SHT_RELA");
			return ERROR_INVALID_FORMAT;
#endif
			assert("Architecture not supported");
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
												(char *)kernel_reserve_range.string_va_start, &string_table[ symbol_table[sym_index].st_name ] );
				if ( symbol  )
					symbol_value = symbol->st_value;
				else
				{
					KTRACE("Symbol not found %s", &string_table[ symbol_table[sym_index].st_name ] );
					return ERROR_SYMBOL_NOT_FOUND;
				}
			}
			else
			{
				symbol_value = section_loaded_va[symbol_table[sym_index].st_shndx] + symbol_table[sym_index].st_value;
			}
#if ARCH == i386
			if ( section_loaded_va[relocation_section_index] == NULL )
			{
				kprintf("relocation_section_index=%d\n", relocation_section_index);
				panic("section_loaded_va[relocation_section_index] == NULL");
			}
			RelocateI386Field(rp, symbol_value, section_loaded_va[relocation_section_index], type);
#endif
		}
	}
	return ERROR_SUCCESS;
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
			KTRACE("ELF - Unsupported relocation entry type");
			break;
		default:
			KTRACE("ELF - Unknown relocation entry type");
	}
}
#endif
