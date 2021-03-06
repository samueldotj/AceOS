; trampoline.asm
;Copied from Linux source
;Real mode 16 bit code for starting secondary processors.

%include "/kernel/i386/i386.inc"

EXTERN SecondaryCPUEntry

GLOBAL trampoline_data
GLOBAL trampoline_end

[BITS 16]
[SECTION .text]
trampoline_data:

	wbinvd															; Needed for NUMA-Q should be harmless for others
	
	mov	ax, cs														; Code and data in the same place
	mov	ds, ax
	
	shl eax, 4														;left shift the segment to get physical address
	add eax, (KERNEL_VIRTUAL_ADDRESS - KERNEL_PHYSICAL_ADDRESS)		;convert the physical address to virtual address
	add eax, KSTACK_SIZE											;stack starts from bottom
	mov esp, eax													;this code area is used as stack later
	
	cli																; We should be safe anyway

	mov dword [0], 0xA5A5A5A5 										; write marker for master knows we're running

	;GDT tables in non default location kernel can be beyond 16MB and lgdt will not be able to load the address as in real mode default
	;operand size is 16bit. Use lgdtl instead to force operand size to 32 bit.

	lidt [ds:boot_idt_descr-trampoline_data] 						; load idt with 0, 0
	lgdt [ds:boot_gdt_descr-trampoline_data] 						; load gdt with whatever is appropriate

	mov eax, cr0				        								
    or 	eax, 1b														; protected mode (PE) bit
    mov cr0, eax													; switching to PMode
	
	mov ax, KERNEL_DATA_SELECTOR
	mov ds, ax
	mov es, ax
	mov gs, ax
	mov fs, ax
	mov ss, ax
	
	;flush prefetch and jump to 32 bit SecondaryCPUEntry routine
	jmp	KERNEL_CODE_SELECTOR: dword KERNEL_BOOT_ADDRESS(SecondaryCPUEntry)

;These need to be in the same 64K segment as the above; hence we don't use the gdt defined in gdt.c
align 16
boot_gdt_descr:
	dw   GDT_ENTRIES*8- 1             								; gdt limit
	dd   KERNEL_BOOT_ADDRESS(gdt)     								; gdt base
	
align 16
boot_idt_descr:
	dw   IDT_ENTRIES*8-1                               				; idt limit
	dd   KERNEL_BOOT_ADDRESS(idt)                       			; idt base

trampoline_end:
