define ma
	set $ma_count = memory_area_count
	set $mas = memory_areas
	set $i = 0
	printf "%d memory area(s) at %p\n", $ma_count, $mas
	while $i < $ma_count
		set $pmr_count = $mas[$i]->physical_memory_regions_count
		set $pmr = $mas[$i]->physical_memory_regions
		printf " %d physical memory region(s) at %p\n", $pmr_count, $pmr
		set $j = 0
			printf "\t      Start        End   vp_array   vp_count\n"
		while $j < $pmr_count
			printf "\t %10p %10p %10p %10d\n", $pmr[$j]->start_physical_address, $pmr[$j]->end_physical_address, $pmr[$j]->virtual_page_array, $pmr[$j]->virtual_page_count
			set $j = $j + 1
		end
		set $i = $i + 1
	end
end

document ma
	Prints memory area information
end

define kmem
	printf "yet to implement"
end
document kmem
	Prints kernel memory allocation information
end

define pte
	if $argc == 0
		printf "Usage: pte <pte_value>\n" 
	end

	set $pte = (PAGE_TABLE_ENTRY)$arg0
	printf "PTE(0x%x) : %d => %p", $pte->all, $pte->_.page_pfn, PFN_TO_PA($pte->_.page_pfn)
	if $pte->_.present == 0
		printf " NP \n"
	else
		if $pte->_.supervisior
			printf " K "
		else
			printf " U "
		end
		if $pte->_.write
			printf " RW "
		else
			printf " RO "
		end
		if $pte->_.global
			printf " G "
		end
		
		if $pte->_.write_through
			printf " WT "
		else
			printf " WB "
		end
		if $pte->_.cache_disabled
			printf " C "
		end
		if $pte->_.accessed
			printf " D "
		end
		printf "\n"
	end
end
document pte
	Prints the page table entry in human readable format
end
	


define pde
	if $argc == 0
		printf "Usage: pde <pde_value>\n" 
	end

	set $pde = (PAGE_DIRECTORY_ENTRY)$arg0
	printf "PDE(0x%x) : %d => %p", $pde->all, $pde->_.page_table_pfn, PFN_TO_PA($pde->_.page_table_pfn)
	
	if $pde->_.present == 0
		printf " NP \n"
	else
		if $pde->_.supervisior
			printf " K "
		else
			printf " U "
		end
		if $pde->_.write
			printf " RW "
		else
			printf " RO "
		end
		if $pde->_.global
			printf " G "
		end
		
		if $pde->_.write_through
			printf " WT "
		else
			printf " WB "
		end
		if $pde->_.cache_disabled
			printf " C "
		end
		if $pde->_.accessed
			printf " D "
		end
		if $pde->_.page_size == 0
			printf " 4K "
		else
			printf " %d ", $pde->_.page_size
		end
		printf "\n"
	end
end
document pde
	Prints the page directory entry in human readable format
end

define v2p
	if $argc == 0
		printf "Usage: v2p <virtual_address>\n" 
	end
	set $va = $arg0
	set $pde = ((PAGE_DIRECTORY_ENTRY_PTR)PT_SELF_MAP_PAGE_DIRECTORY)[ PAGE_DIRECTORY_ENTRY_INDEX($va) ]
	if $pde->_.present
		set $pte = PT_SELF_MAP_PAGE_TABLE1_PTE( $va )
	else
		set $pte = NULL
	end
	if $pte->_.present
		set $pa = PFN_TO_PA( $pte->_.page_pfn )
	else
		set $pa = 0
	end
	printf "Page Directory %p Page Directory Index %d\n", PT_SELF_MAP_PAGE_DIRECTORY, PAGE_DIRECTORY_ENTRY_INDEX($va)
	printf "Page Table     %p Page Table Index     %d PTE %p\n", PT_SELF_MAP_PAGE_TABLE1($va), PAGE_TABLE_ENTRY_INDEX($va),  PT_SELF_MAP_PAGE_TABLE1_PTE($va)
	printf "---------------------------------\n"
	printf "VIRTUAL ADDRESS  PHYSICAL ADDRESS\n"
	printf "%p \t \t %p\n", $va, $pa
	printf "---------------------------------\n"
	pde $pde->all
	pte $pte->all
end
document v2p
	Prints virtual to physical address translation
	Usage: v2p <virtual_address>
end

