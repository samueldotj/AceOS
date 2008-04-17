;This file is all about interrupts and exceptions.
;author: DilipSimha N M
;date: 11-OCT-2007 4:45pm

;	We call a C function in here. We need to let the assembler know that '_fault_handler' exists in another file.
extern ExceptionHandler
extern InterruptHandler

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
		mov eax, ExceptionHandler
	%else
		mov eax, InterruptHandler
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
global InterruptStub0
global InterruptStub1
global InterruptStub2
global InterruptStub3
global InterruptStub4
global InterruptStub5
global InterruptStub6
global InterruptStub7
global InterruptStub8
global InterruptStub9
global InterruptStub10
global InterruptStub11
global InterruptStub12
global InterruptStub13
global InterruptStub14
global InterruptStub15


; define the interrupt handlers here.
global ExceptionStub0
global ExceptionStub1
global ExceptionStub2
global ExceptionStub3
global ExceptionStub4
global ExceptionStub5
global ExceptionStub6
global ExceptionStub7
global ExceptionStub8
global ExceptionStub9
global ExceptionStub10
global ExceptionStub11
global ExceptionStub12
global ExceptionStub13
global ExceptionStub14
global ExceptionStub15
global ExceptionStub16
global ExceptionStub17
global ExceptionStub18
global ExceptionStub19
global ExceptionStub20
global ExceptionStub21
global ExceptionStub22
global ExceptionStub23
global ExceptionStub24
global ExceptionStub25
global ExceptionStub26
global ExceptionStub27
global ExceptionStub28
global ExceptionStub29
global ExceptionStub30
global ExceptionStub31

;	Exceptions

;	0: Divide By Zero Exception
ExceptionStub0:
	IsrStubMacro e, 0, 0

;	1: Debug Exception
ExceptionStub1:
	IsrStubMacro e, 0, 1
    
;	2: Non Maskable Interrupt Exception
ExceptionStub2:
	IsrStubMacro e, 0, 2

;	3: Breakpoint Exception
ExceptionStub3:
	IsrStubMacro e, 0, 3

;	4: Into Detected Overflow Exception
ExceptionStub4:
	IsrStubMacro e, 0, 4

;	5: Out of Bounds Exception
ExceptionStub5:
	IsrStubMacro e, 0, 5

;	6: Invalid Opcode Exception
ExceptionStub6:
	IsrStubMacro e, 0, 6

;	7: No Coprocessor Exception
ExceptionStub7:
	IsrStubMacro e, 0, 7

;	8: Double Fault Exception
ExceptionStub8:
	IsrStubMacro e, 1, 8

;	9: Coprocessor Segment Overrun Exception
ExceptionStub9:
	IsrStubMacro e, 0, 9

;	10: Bad TSS Exception
ExceptionStub10:
	IsrStubMacro e, 1, 10

;	11: Segment Not Present Exception
ExceptionStub11:
	IsrStubMacro e, 1, 11

;	12: Stack Fault Exception
ExceptionStub12:
	IsrStubMacro e, 1, 12

;	13: General Protection Fault Exception
ExceptionStub13:
	IsrStubMacro e, 1, 13

;	14: Page Fault Exception
ExceptionStub14:
	IsrStubMacro e, 1, 14

;	15: Unknown Interrupt Exception
ExceptionStub15:
	IsrStubMacro e, 0, 15

;	16: Coprocessor Fault Exception
ExceptionStub16:
	IsrStubMacro e, 0, 16

;	17: Alignment Check Exception (486+)
ExceptionStub17:
	IsrStubMacro e, 0, 17

;	18: Machine Check Exception (Pentium/586+)
ExceptionStub18:
	IsrStubMacro e, 0, 18

;	19: Reserved Exceptions
ExceptionStub19:
	IsrStubMacro e, 0, 19

;	20: Reserved Exceptions
ExceptionStub20:
	IsrStubMacro e, 0, 20

;	21: Reserved Exceptions
ExceptionStub21:
	IsrStubMacro e, 0, 21

;	22: Reserved Exceptions
ExceptionStub22:
	IsrStubMacro e, 0, 22

;	23: Reserved Exceptions
ExceptionStub23:
	IsrStubMacro e, 0, 23

;	24: Reserved Exceptions
ExceptionStub24:
	IsrStubMacro e, 0, 24

;	25: Reserved Exceptions
ExceptionStub25:
	IsrStubMacro e, 0, 25

;	26: Reserved Exceptions
ExceptionStub26:
	IsrStubMacro e, 0, 26

;	27: Reserved Exceptions
ExceptionStub27:
	IsrStubMacro e, 0, 27

;	28: Reserved Exceptions
ExceptionStub28:
	IsrStubMacro e, 0, 28

;	29: Reserved Exceptions
ExceptionStub29:
	IsrStubMacro e, 0, 29

;	30: Reserved Exceptions
ExceptionStub30:
	IsrStubMacro e, 0, 30

;	31: Reserved Exceptions
ExceptionStub31:
	IsrStubMacro e, 0, 31


;	Interrupts

;	32: IRQ0
InterruptStub0:
	IsrStubMacro i, 0, 32

;	33: IRQ1
InterruptStub1:
	IsrStubMacro i, 0, 33

;	34: IRQ2
InterruptStub2:
	IsrStubMacro i, 0, 34

;	35: IRQ3
InterruptStub3:
	IsrStubMacro i, 0, 35

;	36: IRQ4
InterruptStub4:
	IsrStubMacro i, 0, 36

;	37: IRQ5
InterruptStub5:
	IsrStubMacro i, 0, 37

;	38: IRQ6
InterruptStub6:
	IsrStubMacro i, 0, 38

;	39: IRQ7
InterruptStub7:
	IsrStubMacro i, 0, 39

;	40: IRQ8
InterruptStub8:
	IsrStubMacro i, 0, 40

;	41: IRQ9
InterruptStub9:
	IsrStubMacro i, 0, 41

;	42: IRQ10
InterruptStub10:
	IsrStubMacro i, 0, 42

;	43: IRQ11
InterruptStub11:
	IsrStubMacro i, 0, 43

;	44: IRQ12
InterruptStub12:
	IsrStubMacro i, 0, 44

;	45: IRQ13
InterruptStub13:
	IsrStubMacro i, 0, 45

;	46: IRQ14
InterruptStub14:
	IsrStubMacro i, 0, 46

;	47: IRQ15
InterruptStub15:
	IsrStubMacro i, 0, 47

