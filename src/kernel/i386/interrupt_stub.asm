; Interrupt and exception stubs

%include "kernel/i386/i386.inc"

extern ExceptionHandler
extern PageFaultHandler
extern InterruptHandler
extern SetIdtGate
global ReturnFromInterruptContext

global SetupInterruptStubs
global SetupExceptionStubs

; Common ISR stub. It saves the processor state, sets up for kernel mode segments, calls the C-level fault handler and finally restores the stack frame.
; For some exceptions, processor pushes the error code by default, for other cases this stub will push error code as 0.
; Arguments
;	Handler - 	Function address
;	IntNo	- 	Interrupt number
%macro IsrStubMacro 2
global %1Stub%2

%1Stub%2:

	%if (%2 < 8) || (%2 == 9) || (%2 > 14)					; Push dummy error code if required
		push dword 0
	%endif
	push dword %2											; Push interrupt number
	
	pusha													; Save general purpose registers
	push ds
	push es
	push fs
	push gs
	
	mov eax, cr3											; Save control registers
	push eax
	mov eax, cr2
	push eax
	;mov eax, cr1
	push dword 0
	mov eax, cr0
	push eax
	
	mov ax, KERNEL_DATA_SELECTOR							; Load the Kernel Data Segment descriptor into segment registers
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, esp											; Push the stack pointer
	push eax
	;call the exception/interrupt handler
	mov eax, %1
	call eax												; call Interrupt/Exception handler(pushes the EIP)
	
	jmp ReturnFromInterruptContext
	
%endmacro

ReturnFromInterruptContext:	
	pop eax													
	
	add esp, 16												; Clean up the pushed control registers
	
	pop gs													; Pop segment registers
	pop fs
	pop es
	pop ds
	popa													; Pop general purpose registers
	
	add esp, 8												; Pop interrupt number and error code
	
	iret													; return from interrupt context - pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!
	

;Install the stub as interrupt gates
;Parameters
%macro SetIdtGateMacro 2
	push dword %1%2
	push dword %2
	call SetIdtGate
	add esp, 8
%endmacro

[SECTION .text]
[BITS 32]

;Generate exception stubs
%assign i 0
%rep 32
	%if i = 14
		IsrStubMacro PageFaultHandler, i
	%else
		IsrStubMacro ExceptionHandler, i
	%endif
	%assign i i+1
%endrep

;Generate interrupt stubs
%assign i 32
%rep 255-32
	IsrStubMacro InterruptHandler, i
	%assign i i+1
%endrep

SetupExceptionStubs:
	%assign i 0
	%rep 32
		%if i = 14
			SetIdtGateMacro PageFaultHandlerStub, i
		%else
			SetIdtGateMacro ExceptionHandlerStub, i
		%endif
		
		%assign i i+1
	%endrep
	ret

SetupInterruptStubs:
	%assign i 32
	%rep 255-32
		SetIdtGateMacro InterruptHandlerStub, i
		
		%assign i i+1
	%endrep
	ret