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

EXTERN _sbss
EXTERN _ebss
EXTERN _cmain
EXTERN _InitKernelPageDirectory

GLOBAL _KernelEntry

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
	dd _sbss - KERNEL_VIRTUAL_ADDRESS + KERNEL_PHYSICAL_ADDRESS			; end of kernel .data section
	dd _ebss - KERNEL_VIRTUAL_ADDRESS + KERNEL_PHYSICAL_ADDRESS			; end of kernel BSS
	dd _KernelEntry - KERNEL_VIRTUAL_ADDRESS + KERNEL_PHYSICAL_ADDRESS	; kernel entry point (initial EIP)

align 4
_KernelEntry:
	;create kernel stack
	mov esp, ((kstack+KSTACK_SIZE) - KERNEL_VIRTUAL_ADDRESS) + KERNEL_PHYSICAL_ADDRESS

	push ebx                            ;Push the pointer to the Multiboot information structure.
	push eax                            ;Push the magic value

	; Zeroing BSS section
	mov edi, _sbss
	mov ecx, _ebss
	sub ecx, edi
	add edi, KERNEL_PHYSICAL_ADDRESS - KERNEL_VIRTUAL_ADDRESS
	xor eax, eax
	rep stosb
	
	;setup kernel page directory
	call _InitKernelPageDirectory
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
	;parameters are pushed initially
	call _cmain

	;endless loop should not be reached.
	jmp $

align 4
[SECTION .data]
	kstack times KSTACK_SIZE dd 0

