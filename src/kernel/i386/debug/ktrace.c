/*!
	\file		ktrace.c
	\author		Samuel
	\version	3.0
	\date	
  			Created: 26/09/07 11:28
  			Last modified: 26/09/07 11:28
	\brief	i386 specific ktrace output
*/

#include <ace.h>
#include <kernel/debug.h>
#include <kernel/i386/serial.h>
#include <kernel/i386/vga_text.h>

//#define KTRACE_PRINT_SERIAL
/*if serial port is enabled then the port register*/
#ifdef KTRACE_PRINT_SERIAL
	#define KTRACE_SERIAL_PORT 0x3F8
#endif

#define KTRACE_PRINT_VGA

/*!Writes the given character to serial/vga

This function is pointed by ktrace_putc function pointer.
ktrace_putc function pointer used by ktrace() function defined in printf.c
ktrace function is used by KTRACE() macro.
*/
void KtracePrint(BYTE ch)
{
	#ifdef KTRACE_PRINT_SERIAL
		SerialWriteCharacter(KTRACE_SERIAL_PORT, ch);
	#endif
	#ifdef KTRACE_PRINT_VGA
		VgaPrintCharacter(ch);
	#endif
}

/*!Intialize the ktrace functionality
*/
void InitKtrace()
{
	#ifdef KTRACE_PRINT_SERIAL
		InitSerialPort(KTRACE_SERIAL_PORT, 115200, 8, UART_PARITY_NONE, UART_STOP_BIT_1)
	#endif
	ktrace_putc = KtracePrint;
}
