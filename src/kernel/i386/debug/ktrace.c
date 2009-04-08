/*!
	\file	kernel/i386/debug/ktrace.c
	\brief	i386 specific ktrace output
*/

#include <ace.h>
#include <kernel/debug.h>
#include <kernel/i386/serial.h>
#include <kernel/i386/parallel.h>
#include <kernel/i386/vga_text.h>
#include <kernel/pm/elf.h>

#define KTRACE_PRINT_PARALLEL
#define KTRACE_PRINT_SERIAL
//#define KTRACE_PRINT_VGA

/*if serial port printing is enabled then register port */
#ifdef KTRACE_PRINT_SERIAL
	#define KTRACE_SERIAL_PORT 0x3F8
#endif
/*if parallel port printing is enabled then register port*/
#ifdef KTRACE_PRINT_PARALLEL
	#define KTRACE_PARALLEL_PORT 0x378
#endif

/*! Writes the given character to serial/vga

This function is pointed by ktrace_putc function pointer.
ktrace_putc function pointer used by ktrace() function defined in printf.c
ktrace function is used by KTRACE() macro.
*/
void KtracePrint(void * arg, char ch)
{
	#ifdef KTRACE_PRINT_VGA
		VgaPrintCharacter(ch);
	#endif
	#ifdef KTRACE_PRINT_PARALLEL
		WriteParallelPort(KTRACE_PARALLEL_PORT, ch);
	#endif
	#ifdef KTRACE_PRINT_SERIAL
		SerialWriteCharacter(KTRACE_SERIAL_PORT, ch);
	#endif
}

/*! Intialize the ktrace functionality
*/
void InitKtrace()
{
	#ifdef KTRACE_PRINT_PARALLEL
		InitParallelPort(KTRACE_PARALLEL_PORT);
	#endif
	#ifdef KTRACE_PRINT_SERIAL
		InitSerialPort(KTRACE_SERIAL_PORT, 115200, 8, UART_PARITY_NONE, UART_STOP_BIT_1);
	#endif
	
	ktrace_putc = KtracePrint;
}

/*! Prints the current call stack trace
	\param max_frames - maximum frames to print

Stack frame contains:
    Function arguments
    Return address of calling function
    ebp of calling function
*/
void PrintStackTrace(unsigned int max_frames)
{
    unsigned int * ebp;
	unsigned int frame;
	
	ebp = &max_frames - 2;
	
    for(frame = 0; frame<max_frames; ++frame)
    {
        unsigned int * arguments, eip = ebp[1];
		int offset;
		char * func;
        if( ebp[0] < PAGE_SIZE || eip < PAGE_SIZE)
            return;
        
		/* Unwind to previous stack frame*/
        ebp = (unsigned int *)(ebp[0]);
        arguments = &ebp[2];
		func = FindKernelSymbolByAddress(eip, &offset);
        kprintf("%d) %p %s+%p(%p)\n", frame+1, eip, func, offset, arguments);
    }
}
