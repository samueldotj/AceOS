/*! \file serial.c
    \brief Parallel wIOBase read/write functions mostly used for debugging
*/

#include <ace.h>
#include <kernel/io.h>
#include <kernel/debug.h>

#define CONTROL_REGISTER	2
#define STATUS_REGISTER		1

#define MAX_PRINTER_PORTS	4
#define PRINTER_TIME_OUT	10

int LegacyParallelPorts[MAX_PRINTER_PORTS]={0x378,0x278,0,0};

int InitParallelPort(UINT16 wIOBase)
{
	int i, control_reg;
	
    control_reg = _inp(wIOBase+CONTROL_REGISTER);
	/*Clear INIT/BIDIR bits.*/
    control_reg &= 0xDB;
    _outp( wIOBase, control_reg );
	
	/*This will produce at least a 50 usec delay.*/
    for(i=0;i<5;i++);
	
	/*Stop resetting printer.*/
    control_reg |= 0x4;
    _outp( wIOBase, control_reg );
    return 0;
}
int WriteParallelPort(UINT16 wIOBase, BYTE Character)
{
    BOOLEAN Busy=TRUE;
	int i, j, control_reg;
	
    for(i=0;i<PRINTER_TIME_OUT ;i++)
	{
        for(j=0;j<100;j++)
        {
            if ( _inp(wIOBase+STATUS_REGISTER) & 0x80 )
            {
                Busy=FALSE;
                break;
            }
        }
	}
    _outp(wIOBase, Character); 
        
    control_reg = _inp( wIOBase+CONTROL_REGISTER );
    control_reg &= 0x1E;
    _outp( wIOBase+CONTROL_REGISTER, control_reg);
        
    for(i=0;i<16;i++);
    
    control_reg |= 1;
    _outp(wIOBase+CONTROL_REGISTER, control_reg); //strobe
    for(i=0;i<16;i++);
        
    control_reg &= 0xFE;
    _outp(wIOBase+CONTROL_REGISTER, control_reg); //clear strobe
        
    return 0;
}
