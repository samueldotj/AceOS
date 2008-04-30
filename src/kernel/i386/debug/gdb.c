/*! \file gdb.c
    \author Samuel (samueldotj@gmail.com)
    \date 20-Jan-07 11:19

    \brief GDB interface function definitions
	This file contains all the routines which are required by the gdb-stub.c
*/
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/i386/serial.h>
#include <kernel/i386/idt.h>

/*! This port can be configured through kernel boot parameters*/
UINT32 sys_gdb_port = 0;

void InitGdb()
{
	/*set up the serial port*/
	InitSerialPort(sys_gdb_port, 115200, UART_DATA_BIT_8, UART_PARITY_NONE, UART_STOP_BIT_1);
	/*setup exception handlers*/
	set_debug_traps();
	kprintf("Waiting for GDB(0x%X) : ", sys_gdb_port );
	/*break point exception to sync with GDB host*/
	__asm__("int3");
	kprintf("connected\n");
}

inline int getDebugChar(void)
{
	return SerialReadCharacter( sys_gdb_port );
}

inline void putDebugChar(int ch)
{
	SerialWriteCharacter( sys_gdb_port, ch );
}

void exceptionHandler(int exc, void *addr)
{
	if (exc != 14 &&    /* page fault (normal) */
		exc != 13 &&    /* GPF (normal in V86 mode) */
		exc != 8 )     /* double fault (so abnormal we just crash) */
	{
		SetIdtGate(exc, (UINT32)addr);
	}
}

void flush_i_cache(void)
{
     __asm__ __volatile__ ("jmp 1f\n1:");
}

