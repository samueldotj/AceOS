/*! \file serial.c

    \brief Serial read/write functions mostly used for remote debugging with GDB
*/

#include <ace.h>
#include <kernel/io.h>
#include <kernel/debug.h>

#define UART_BUFFER				0
#define UART_IER				1   //Interrrupt Enable register (read/write)
#define UART_IIR				2   //Interrupt Identification register (read only)
#define UART_FCR				2   //FIFO Control register(write only)
#define UART_LCR				3   //Line Control register(read/write)
#define UART_MCR				4   //Modem Control register(read/write)
#define UART_LSR				5   //Line status register (read only)
#define UART_MSR				6   //Modem status register (read only)
#define UART_SCRATCH			7   

#define UART_DIVISOR_LATCH_LOW  0
#define UART_DIVISOR_LATCH_HIGH 1

#define MAX_SERIAL_PORTS		4
int LegacySerialPorts[MAX_SERIAL_PORTS]={0x3F8, 0x2F8, 0x3E8, 0x2E8};

/*! Initializes the given serial port with the given properties.

1. Calculate divisor value based on BaudRate
2. Calculate controlbyte based on Partity, Stopbit and Databits
3. Set the UART in DLAB mode
4. Send the DLAB high and low values
5. Set the UART control value.
*/
void InitSerialPort(UINT16 wIOBase, UINT32 wBaudRate, BYTE DataBits, BYTE Parity, BYTE StopBit)
{
	UINT16 wDivisor;
	BYTE ControlByte = 0;

	/*set the default baud rate if it is not specified*/     
	if ( wBaudRate == 0 )
		wBaudRate = 9600;

	/*UART's clock frequency 1.8432 MHZ*/
	wDivisor = 1843200 / (wBaudRate * 16);

	/*we need only 3 bits, and move it to bit position 3-5*/
	Parity &= 0x7;
	Parity <<= 3;
	/*only 2 bits, and it should be in 0-1*/
	DataBits &= 0x3;
	
	ControlByte = DataBits | Parity;
	if ( StopBit )
	ControlByte |= 0x2;
	
	/*set Divisor Latch Access mode*/
	_outp( wIOBase + UART_LCR, 0x80 );
	
	/*set the UART speed*/
	_outp( wIOBase + UART_DIVISOR_LATCH_LOW, wDivisor % 256 );
	_outp( wIOBase + UART_DIVISOR_LATCH_HIGH, wDivisor / 256 );
	
	/*set Line Control Register*/
	_outp( wIOBase + UART_LCR, ControlByte );
}

/*! Receives and returns a character from the given serial port*/
char SerialReadCharacter(UINT16 wIOBase)
{
	while (1)
	{
		/*read line status register*/
		BYTE in = _inp( wIOBase + UART_LSR );
		/*if data ready read the data and return.*/
		if ( in & 0x1 )
		{
			return _inp( wIOBase );
		}
	}
}

/*! Sends the given character to the given serial port*/
void SerialWriteCharacter(UINT16 wIOBase, char ch)
{
	while (1)
	{
		/*read line status register*/
		BYTE in = _inp( wIOBase + UART_LSR );
		/*if transmitter holding register is ready, write the data */
		if ( in & 0x20 ) 
		{
			_outp( wIOBase, ch );
			return;
		}
	}
}
