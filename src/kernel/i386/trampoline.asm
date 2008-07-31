; trampoline.asm
;Copied from Linux source
;Real mode 16 bit code for starting secondary processors.

KERNEL_CODE_SELECTOR	equ		0x8
GDT_ENTRIES				equ		0x5

KERNEL_PHYSICAL_ADDRESS	equ 	0x100000
KERNEL_VIRTUAL_ADDRESS  equ 	(0xC0000000 + KERNEL_PHYSICAL_ADDRESS)

%define KERNEL_BOOT_ADDRESS(va)	(va- KERNEL_VIRTUAL_ADDRESS + KERNEL_PHYSICAL_ADDRESS)

EXTERN	gdt
EXTERN	SecondaryCPUEntry

[BITS 16]
[SECTION .boot]
GLOBAL trampoline_data
GLOBAL trampoline_end
trampoline_data:

	wbinvd										; Needed for NUMA-Q should be harmless for others
	mov	ax, cs									; Code and data in the same place
	mov	ds, ax
	
	shl eax, 4								;left shift the segment to get physical address
	mov esp, eax								;this code area is used as stack later

	cli											; We should be safe anyway

	mov dword [0], 0xA5A5A5A5 					; write marker for master knows we're running

	;GDT tables in non default location kernel can be beyond 16MB and lgdt will not be able to load the address as in real mode default
	;operand size is 16bit. Use lgdtl instead to force operand size to 32 bit.

	lidt	[boot_idt_descr-trampoline_data] 	; load idt with 0, 0
	lgdt	[boot_gdt_descr-trampoline_data] 	; load gdt with whatever is appropriate

	xor		ax, ax
	inc		ax									; protected mode (PE) bit
	lmsw	ax									; into protected mode
	
	;flush prefetch and jump to 32 bit StartSecondaryProcessor() C function
	jmp	dword KERNEL_CODE_SELECTOR: KERNEL_BOOT_ADDRESS(SecondaryCPUEntry)


;These need to be in the same 64K segment as the above; hence we don't use the gdt defined in gdt.c
align 16
boot_gdt_descr:
	dw   GDT_ENTRIES*8- 1             			; gdt limit
	dd   KERNEL_BOOT_ADDRESS(gdt)     			; gdt base
	
align 16
boot_idt_descr:
	dw   0                               	; idt limit = 0
	dd   0                               	; idt base = 0L

trampoline_end:
