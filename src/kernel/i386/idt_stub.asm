;This file is all about interrupts and exceptions.
;author: DilipSimha N M
;date: 11-OCT-2007 4:45pm

;	We call a C function in here. We need to let the assembler know that '_fault_handler' exists in another file.
extern _ExceptionHandler
extern _InterruptHandler

;	This is our common ISR stub. It saves the processor state, sets up for kernel mode segments, 
;	calls the C-level fault handler and finally restores the stack frame.
;	This will push error number and exception number on the stack. 
;	For some exceptions, processor pushes the error code by default.
;	So we need to check for that case and push appropriately.
%macro IsrStubMacro 3
	cli
	%ifidni %2,0 
          push byte %2
	%endif
	push byte %3
	pusha
	push ds
	push es
	push fs
	push gs
	mov ax, 0x10   ; Load the Kernel Data Segment descriptor!
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, esp   ; Push us the stack
	push eax
	%ifidni %1,e 
		mov eax, _ExceptionHandler
	%else
		mov eax, _InterruptHandler
	%endif
	call eax       ; A special call, preserves the 'eip' register
	pop eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8		; Cleans up the pushed error code and pushed ISR number.
	iret			; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!
%endmacro


; From now, it's all about interrupts
global _InterruptStub0
global _InterruptStub1
global _InterruptStub2
global _InterruptStub3
global _InterruptStub4
global _InterruptStub5
global _InterruptStub6
global _InterruptStub7
global _InterruptStub8
global _InterruptStub9
global _InterruptStub10
global _InterruptStub11
global _InterruptStub12
global _InterruptStub13
global _InterruptStub14
global _InterruptStub15


; define the interrupt handlers here.
global _ExceptionStub0
global _ExceptionStub1
global _ExceptionStub2
global _ExceptionStub3
global _ExceptionStub4
global _ExceptionStub5
global _ExceptionStub6
global _ExceptionStub7
global _ExceptionStub8
global _ExceptionStub9
global _ExceptionStub10
global _ExceptionStub11
global _ExceptionStub12
global _ExceptionStub13
global _ExceptionStub14
global _ExceptionStub15
global _ExceptionStub16
global _ExceptionStub17
global _ExceptionStub18
global _ExceptionStub19
global _ExceptionStub20
global _ExceptionStub21
global _ExceptionStub22
global _ExceptionStub23
global _ExceptionStub24
global _ExceptionStub25
global _ExceptionStub26
global _ExceptionStub27
global _ExceptionStub28
global _ExceptionStub29
global _ExceptionStub30
global _ExceptionStub31

;	Exceptions

;	0: Divide By Zero Exception
_ExceptionStub0:
	IsrStubMacro e, 0, 0

;	1: Debug Exception
_ExceptionStub1:
	IsrStubMacro e, 0, 1
    
;	2: Non Maskable Interrupt Exception
_ExceptionStub2:
	IsrStubMacro e, 0, 2

;	3: Breakpoint Exception
_ExceptionStub3:
	IsrStubMacro e, 0, 3

;	4: Into Detected Overflow Exception
_ExceptionStub4:
	IsrStubMacro e, 0, 4

;	5: Out of Bounds Exception
_ExceptionStub5:
	IsrStubMacro e, 0, 5

;	6: Invalid Opcode Exception
_ExceptionStub6:
	IsrStubMacro e, 0, 6

;	7: No Coprocessor Exception
_ExceptionStub7:
	IsrStubMacro e, 0, 7

;	8: Double Fault Exception
_ExceptionStub8:
	IsrStubMacro e, 1, 8

;	9: Coprocessor Segment Overrun Exception
_ExceptionStub9:
	IsrStubMacro e, 0, 9

;	10: Bad TSS Exception
_ExceptionStub10:
	IsrStubMacro e, 1, 10

;	11: Segment Not Present Exception
_ExceptionStub11:
	IsrStubMacro e, 1, 11

;	12: Stack Fault Exception
_ExceptionStub12:
	IsrStubMacro e, 1, 12

;	13: General Protection Fault Exception
_ExceptionStub13:
	IsrStubMacro e, 1, 13

;	14: Page Fault Exception
_ExceptionStub14:
	IsrStubMacro e, 1, 14

;	15: Unknown Interrupt Exception
_ExceptionStub15:
	IsrStubMacro e, 0, 15

;	16: Coprocessor Fault Exception
_ExceptionStub16:
	IsrStubMacro e, 0, 16

;	17: Alignment Check Exception (486+)
_ExceptionStub17:
	IsrStubMacro e, 0, 17

;	18: Machine Check Exception (Pentium/586+)
_ExceptionStub18:
	IsrStubMacro e, 0, 18

;	19: Reserved Exceptions
_ExceptionStub19:
	IsrStubMacro e, 0, 19

;	20: Reserved Exceptions
_ExceptionStub20:
	IsrStubMacro e, 0, 20

;	21: Reserved Exceptions
_ExceptionStub21:
	IsrStubMacro e, 0, 21

;	22: Reserved Exceptions
_ExceptionStub22:
	IsrStubMacro e, 0, 22

;	23: Reserved Exceptions
_ExceptionStub23:
	IsrStubMacro e, 0, 23

;	24: Reserved Exceptions
_ExceptionStub24:
	IsrStubMacro e, 0, 24

;	25: Reserved Exceptions
_ExceptionStub25:
	IsrStubMacro e, 0, 25

;	26: Reserved Exceptions
_ExceptionStub26:
	IsrStubMacro e, 0, 26

;	27: Reserved Exceptions
_ExceptionStub27:
	IsrStubMacro e, 0, 27

;	28: Reserved Exceptions
_ExceptionStub28:
	IsrStubMacro e, 0, 28

;	29: Reserved Exceptions
_ExceptionStub29:
	IsrStubMacro e, 0, 29

;	30: Reserved Exceptions
_ExceptionStub30:
	IsrStubMacro e, 0, 30

;	31: Reserved Exceptions
_ExceptionStub31:
	IsrStubMacro e, 0, 31


;	Interrupts

;	32: IRQ0
_InterruptStub0:
	IsrStubMacro i, 0, 32

;	33: IRQ1
_InterruptStub1:
	IsrStubMacro i, 0, 33

;	34: IRQ2
_InterruptStub2:
	IsrStubMacro i, 0, 34

;	35: IRQ3
_InterruptStub3:
	IsrStubMacro i, 0, 35

;	36: IRQ4
_InterruptStub4:
	IsrStubMacro i, 0, 36

;	37: IRQ5
_InterruptStub5:
	IsrStubMacro i, 0, 37

;	38: IRQ6
_InterruptStub6:
	IsrStubMacro i, 0, 38

;	39: IRQ7
_InterruptStub7:
	IsrStubMacro i, 0, 39

;	40: IRQ8
_InterruptStub8:
	IsrStubMacro i, 0, 40

;	41: IRQ9
_InterruptStub9:
	IsrStubMacro i, 0, 41

;	42: IRQ10
_InterruptStub10:
	IsrStubMacro i, 0, 42

;	43: IRQ11
_InterruptStub11:
	IsrStubMacro i, 0, 43

;	44: IRQ12
_InterruptStub12:
	IsrStubMacro i, 0, 44

;	45: IRQ13
_InterruptStub13:
	IsrStubMacro i, 0, 45

;	46: IRQ14
_InterruptStub14:
	IsrStubMacro i, 0, 46

;	47: IRQ15
_InterruptStub15:
	IsrStubMacro i, 0, 47

