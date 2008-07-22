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