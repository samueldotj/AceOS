define kparameters 
	printf "kernel command line: %s\n", sys_kernel_cmd_line
	set $total_parameters = sizeof(kernel_parameters) / sizeof(KERNEL_PARAMETER)
	set $i = 0
	while $i < $total_parameters
		printf "%s at %p = %p\n", kernel_parameters[$i].parameter_name, kernel_parameters[$i].variable_address, *((UINT32)kernel_parameters[$i].variable_address)
		set $i = $i + 1
	end
end
document kparameters
	Prints kernel parameters
end

define lock
	if $argc != 1
		printf "Usage : lock <lock_address>"
	end
	$lock = $arg0
	printf "Lock %p : ", $lock
	if ( $lock & 1 )
		printf "Busy"
	else
		printf "Free"
	end
	printf "\n"
end
document lock
	Prints the given lock details
end