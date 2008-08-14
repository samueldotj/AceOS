/*! \file	kernel/i386/vga_text.h
	\brief	serial port read and write function declarations
*/
#ifndef SERIAL__H
#define SERIAL__H

#include <ace.h>

#define UART_PARITY_NONE        0
#define UART_PARITY_ODD         1
#define UART_PARITY_EVEN        3
#define UART_PARITY_HIGH        5
#define UART_PARITY_LOW         7 

#define UART_STOP_BIT_1         1
#define UART_STOP_BIT_2         2

#define UART_DATA_BIT_5         0
#define UART_DATA_BIT_6         1
#define UART_DATA_BIT_7         2
#define UART_DATA_BIT_8         3

#define UART_FLOW_HARDWARE      0
#define UART_FLOW_NONE          1
#define UART_FLOW_XON           2


#ifdef __cplusplus
	extern "C" {
#endif

void InitSerialPort(UINT16 wIOBase, UINT32 wBaudRate, BYTE DataBits, BYTE Parity, BYTE StopBit);
char SerialReadCharacter(UINT16 wIOBase);
void SerialWriteCharacter(UINT16 wIOBase, char ch);

#ifdef __cplusplus
	}
#endif

#endif
