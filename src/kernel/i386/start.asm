;Ace OS Startup file
;author: Samuel
;date: 26-09-2007 4:02pm

MULTIBOOT_PAGE_ALIGN    equ 1<<0
MULTIBOOT_MEMORY_INFO   equ 1<<1
MULTIBOOT_AOUT_KLUDGE   equ 1<<16

MULTIBOOT_HEADER_MAGIC  equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS  equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
CHECKSUM                equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
VIRTUAL_BASE_ADDRESS    equ 0x100000
PHYSICAL_ADDRESS        equ 0x100000

KSTACK_SIZE             equ 0x2000

EXTERN _sbss
EXTERN _ebss
EXTERN _cmain

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
    dd mboot - VIRTUAL_BASE_ADDRESS + PHYSICAL_ADDRESS          ; these are PHYSICAL addresses
    dd PHYSICAL_ADDRESS                                         ; start of kernel .text (code) section
    dd _sbss - VIRTUAL_BASE_ADDRESS + PHYSICAL_ADDRESS          ; end of kernel .data section
    dd _ebss - VIRTUAL_BASE_ADDRESS + PHYSICAL_ADDRESS          ; end of kernel BSS
    dd _KernelEntry - VIRTUAL_BASE_ADDRESS + PHYSICAL_ADDRESS   ; kernel entry point (initial EIP)

align 4
_KernelEntry:
    mov esp, kstack+KSTACK_SIZE         ;create kernel stack

    push ebx                            ;Push the pointer to the Multiboot information structure.
    push eax                            ;Push the magic value

    ; Zeroing BSS section
    mov edi, _sbss
    mov ecx, _ebss
    sub ecx, edi
    add edi, PHYSICAL_ADDRESS - VIRTUAL_BASE_ADDRESS
    xor eax, eax
    rep stosb

    ;parameters are pushed initially
    call _cmain

    ;endless loop should not be reached.
    jmp $

align 4
[SECTION .data]
    kstack times KSTACK_SIZE dd 0
