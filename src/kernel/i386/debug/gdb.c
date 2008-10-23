/*! \file gdb.c
    \brief GDB interface function definitions
	
	This file contains all the routines which are required by the gdb-stub.c
*/
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/i386/serial.h>
#include <kernel/i386/idt.h>

/*! This port can be configured through kernel boot parameters*/
UINT32 sys_gdb_port = 0;

/*! Initializes the GDB stub*/
void InitGdb()
{
	/*set up the serial port*/
	InitSerialPort(sys_gdb_port, 9600, UART_DATA_BIT_8, UART_PARITY_NONE, UART_STOP_BIT_1);
	/*setup exception handlers*/
	set_debug_traps();
	kprintf("Waiting for GDB(0x%X) : ", sys_gdb_port );
	/*break point exception to sync with GDB host*/
	__asm__("int3");
	kprintf("connected\n");
}
/*! Reads a character from debug port*/
inline int getDebugChar(void)
{
	return SerialReadCharacter( sys_gdb_port );
}
/*! Writes a character to debug port*/
inline void putDebugChar(int ch)
{
	SerialWriteCharacter( sys_gdb_port, ch );
}
/*! Wrapper for GDB exception handler installer*/
void exceptionHandler(int exc, void *addr)
{
	/*dont register page fault with gdb*/
	if ( exc != 14 )
		SetIdtGate(exc, (UINT32)addr);
	
}
/*! Flushes instruction cache*/
void flush_i_cache(void)
{
     __asm__ __volatile__ ("jmp 1f\n1:");
}

