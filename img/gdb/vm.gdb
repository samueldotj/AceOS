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