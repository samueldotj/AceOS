;Ace OS Startup file
;author: Samuel
;date: 26-09-2007 4:02pm

MULTIBOOT_PAGE_ALIGN    equ 1<<0
MULTIBOOT_MEMORY_INFO   equ 1<<1
MULTIBOOT_AOUT_KLUDGE   equ 1<<16

MULTIBOOT_HEADER_MAGIC  equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS  equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
CHECKSUM                equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

KERNEL_PHYSICAL_ADDRESS	equ 0x100000
KERNEL_VIRTUAL_ADDRESS  equ (0xC0000000 + KERNEL_PHYSICAL_ADDRESS)

KSTACK_SIZE             equ 0x2000

EXTERN sbss
EXTERN ebss
EXTERN kernel_page_directory

EXTERN cmain
EXTERN InitPhysicalMemoryManagerPhaseI
EXTERN SecondaryCPUStart

GLOBAL KernelEntry
GLOBAL SecondaryCPUEntry

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
	;create kernel stack //todo this should relocatable
	mov eax, esp
	add eax, (KERNEL_VIRTUAL_ADDRESS - KERNEL_PHYSICAL_ADDRESS)
	mov esp, eax
;init paging
	mov eax, (kernel_page_directory - KERNEL_VIRTUAL_ADDRESS) + KERNEL_PHYSICAL_ADDRESS
	or eax, 0x80000000
	mov cr0, eax

	; flush the prefetch-queue
    jmp .1
.1:
	
	;make sure eip is relocated
	mov eax, .2
	jmp eax
.2:

	call SecondaryCPUStart
	
	;endless loop should not be reached.
	jmp $

align 4
[SECTION .data]
	kstack times KSTACK_SIZE dd 0
