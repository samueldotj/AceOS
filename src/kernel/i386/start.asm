;Ace Kernel Startup file
;32bit Kernel gets control from multiboot loader here.

%include "kernel/i386/i386.inc"

GLOBAL KernelEntry
GLOBAL SecondaryCPUEntry

EXTERN SetupExceptionStubs
EXTERN SetupInterruptStubs

EXTERN idtp
EXTERN gp

[SECTION .boot]
[BITS 32]
;multiboot header
align 4
mboot:
	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd CHECKSUM
	; fields used if MULTIBOOT_AOUT_KLUDGE is set in MULTIBOOT_HEADER_FLAGS
	dd mboot - KERNEL_VIRTUAL_ADDRESS + KERNEL_PHYSICAL_ADDRESS			; these are PHYSICAL addresses
	dd KERNEL_PHYSICAL_ADDRESS											; start of kernel .text (code) section
	dd sbss - KERNEL_VIRTUAL_ADDRESS + KERNEL_PHYSICAL_ADDRESS			; end of kernel .data section
	dd ebss - KERNEL_VIRTUAL_ADDRESS + KERNEL_PHYSICAL_ADDRESS			; end of kernel BSS
	dd KernelEntry - KERNEL_VIRTUAL_ADDRESS + KERNEL_PHYSICAL_ADDRESS	; kernel entry point (initial EIP)

align 4
KernelEntry:
	;create kernel stack
	mov esp, ((kstack+KSTACK_SIZE) - KERNEL_VIRTUAL_ADDRESS) + KERNEL_PHYSICAL_ADDRESS

	push ebx                            ;Push the pointer to the Multiboot information structure.
	push eax                            ;Push the magic value

	; Zeroing BSS section
	mov edi, sbss
	mov ecx, ebss
	sub ecx, edi
	add edi, KERNEL_PHYSICAL_ADDRESS - KERNEL_VIRTUAL_ADDRESS
	xor eax, eax
	rep stosb
	
	;setup kernel page directory
	call InitPhysicalMemoryManagerPhaseI
	
;init paging
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax

	; flush the prefetch-queue
    jmp .1
.1:
	
	;make sure eip is relocated
	mov eax, .2
	jmp eax
.2:

	;load the Global Descriptor Table into GDTR
    call LoadGdt
	;load the Interrupt descriptor table into IDTR
    call LoadIdt
	
	;setup exception handlers and interrupt hanlders
	call SetupExceptionStubs
	call SetupInterruptStubs
	
	;correct stack pointer
	add esp, (KERNEL_VIRTUAL_ADDRESS - KERNEL_PHYSICAL_ADDRESS)
	
	;correct multiboot info pointer
	pop eax
	pop ebx
	add ebx, (KERNEL_VIRTUAL_ADDRESS- KERNEL_PHYSICAL_ADDRESS)
	push ebx
	push eax
	
	call cmain

	;endless loop should not be reached.
	jmp $

	
;this function is called by secondary CPUs while starting
SecondaryCPUEntry:
	;load cr3 with page directory address
	mov eax, KERNEL_BOOT_ADDRESS(kernel_page_directory)
	mov cr3, eax
	
	;set cr4 for 4MB page size and global page
	mov eax, CR4_PAGE_SIZE_EXT | CR4_PAGE_GLOBAL_ENABLE
	mov cr4, eax

	;init paging
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax

	; flush the prefetch-queue
    jmp .1
.1:
	
	;make sure eip is relocated
	mov eax, .2
	jmp eax
.2:
	
	;load the Global Descriptor Table into GDTR
	lidt [ds:idtp]
	;load the Interrupt descriptor table into IDTR
	lgdt [ds:gp]				
	
	call SecondaryCpuStart
	
	;endless loop should not be reached.
	jmp $

[SECTION .bootdata]
align PAGE_SIZE
	kthread		times PAGE_SIZE dd 0
	guard_page 	times PAGE_SIZE dd 0
	kstack 		times KSTACK_SIZE dd 0

