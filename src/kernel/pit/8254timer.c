/*!
  \file		kernel/pit/8254timer.c
  \brief	8254 timer specific functionality.
*/
#include <ace.h>
#include <kernel/io.h>

#define CONTROL_PORT 	(0x43)
#define COUNTER0_PORT 	(0x40)
#define FREQUENCY_8254 	(1193180)


/*!
	\brief	 Initializes 8254 timer
	\param	 frequency - No of oscillator's cycles per second
*/
void Start8254Timer(UINT32 frequency)
{
	UINT32 divisor;

	/* The value we send to the PIT is the value to divide it's input clock
	 * (1193180 Hz) by, to get our required frequency. Important to note is
	 * that the divisor must be small enough to fit into 16-bits.
	 */
	divisor = FREQUENCY_8254 / frequency;

	/* Send the command byte */
	/* mode=SQUARE WAVE and BINARY_COUNTER and Read Write least significant byte first then most significant byte  and select counter 0*/
	_outp(CONTROL_PORT, 0x36);
	
	/* Send the frequency divisor */
	_outp(COUNTER0_PORT, (BYTE)(divisor & 0xFF));
	_outp(COUNTER0_PORT, (BYTE)(divisor>>8));
}
