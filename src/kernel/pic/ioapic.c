/*!
	\file	kernel/pic/ioapic.c
	\brief	Contains IOAPIC and PIC(8259) related routines.
	\todo	Based on _PRT table initiailize the IOAPIC - read http://people.freebsd.org/~jhb/papers/bsdcan/2007/article/node5.html
*/

#include <ace.h>
#include <kernel/error.h>
#include <kernel/debug.h>
#include <kernel/io.h>
#include <kernel/ioapic.h>
#include <kernel/apic.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/virtual_page.h>

/*! Data read/write address from the base address*/
#define IOAPIC_IOWIN_OFFSET		0x10

/*! 8259's master ports*/
#define PIC_MASTER_IO_PORT_A	0x20
#define PIC_MASTER_IO_PORT_B	0x21
/*! 8259's slave ports*/
#define PIC_SLAVE_IO_PORT_A		0xA0
#define PIC_SLAVE_IO_PORT_B		0xA1

/*! If the PIC is wired to different IRQ pin in IOAPIC then there will be entry in this table to indicate there is a redirection*/
UINT32 legacy_irq_redirection_table[16];

IOAPIC ioapic[MAX_IOAPIC];		/*! All IOAPICs in the system are stored here*/
UINT8 count_ioapic;				/*! Total IOAPICs found*/

/*!
 * \brief 	Read data from IOAPIC for the given register.
 * \param   ioapic_base_va		Base address of the IOAPIC
 * \param 	reg					IOAPIC register to be accessed.
 * \param 	data				Pointer to 32 bit memory in which data is filled from IOAPIC.
  */
static void ReadFromIoApic(IOAPIC_REGISTER_SELECTOR_PTR ioapic_base_va, IOAPIC_REGISTER reg, UINT32 *data)
{
	IOAPIC_REGISTER_SELECTOR cmd;
	
	/*! select the register*/
	cmd.dword = 0;
	cmd.ioapic_register = reg;
	ioapic_base_va->dword = cmd.dword;
	
	/*! read the data*/
	*data = *((UINT32 *)(((UINT32)ioapic_base_va) + IOAPIC_IOWIN_OFFSET));
}

/*!
 *	\brief	Write to the specified register in IOAPIC.
 *  \param  ioapic_base_va		Base address of the IOAPIC 
 *	\param	reg - IOAPIC register to be accessed.
 *	\param	data - 32 bit data that has to be written into the specified register.
*/
static void WriteToIoApic(IOAPIC_REGISTER_SELECTOR_PTR ioapic_base_va, IOAPIC_REGISTER reg, UINT32 data)
{
	IOAPIC_REGISTER_SELECTOR cmd;
	
	/*! select the register*/
	cmd.dword = 0;
	cmd.ioapic_register = reg;
	ioapic_base_va->dword = cmd.dword;
	
	/*! write the data*/
	*((UINT32 *)(((UINT32)ioapic_base_va) + IOAPIC_IOWIN_OFFSET))= data;
}

/*! Initializes the Programmable Interrupt Controller 8259
	\param start_vector - The interrupt vector number where the IRQ should raised
	\reference http://www.cs.sun.ac.za/~lraitt/doc_8259.html
*/
void InitPic(BYTE start_vector)
{
	/*! Initialize the master PIC by sending Initialization Command Words 1 to 4*/
	_outp(PIC_MASTER_IO_PORT_A, 0x11);				/* ICW1 - Set expect ICW4 bit*/
	_outp(PIC_MASTER_IO_PORT_B, start_vector);		/* ICW2 - Set the interrupt vector number*/
	_outp(PIC_MASTER_IO_PORT_B, 0x4);				/* ICW3 - Set where the slave is connected*/
	_outp(PIC_MASTER_IO_PORT_B, 0x1);				/* ICW4 - Set operating mode is 8086 and not MCS-80/85*/
	
	_outp(PIC_MASTER_IO_PORT_B, 0x4);				/* Mask cascade interrupt*/
	
	/*! Initialize the slave PIC by sending Initialization Command Words 1-4*/
	_outp(PIC_SLAVE_IO_PORT_A, 0x11);				/* ICW1 - Set expect ICW4 bit*/
	_outp(PIC_SLAVE_IO_PORT_B, start_vector+8);		/* ICW2 - Set the interrupt vector number*/
	_outp(PIC_SLAVE_IO_PORT_B, 0x2);				/* ICW3 - Set where the slave is connected*/
	_outp(PIC_SLAVE_IO_PORT_B, 0x1);				/* ICW4 - Set operating mode is 8086 and not MCS-80/85*/
}

/*! Masks all PIC interrupts from PIC*/
void MaskPic()
{
	_outp(PIC_MASTER_IO_PORT_B, 0xff );
	_outp(PIC_SLAVE_IO_PORT_B, 0xff );
}

/*! Initialize the IOAPIC chip to receive and send interrupts to LAPIC
 *	\param	ioapic_base_va -	Base address of the IOAPIC 
 *  \return total number of irq supported by this ioapic 
 */
int InitIoApic(IOAPIC_REGISTER_SELECTOR_PTR ioapic_base_va, BYTE start_vector)
{
	IOAPIC_REDIRECT_TABLE redirect_table;
	IOAPIC_VERSION ver;
	int i, table_index;
	
	redirect_table.dword_high = redirect_table.dword_low = 0;
	table_index = IOAPIC_REGISTER_REDIRECT_TABLE;
	
	/*! Get the number of maximum redirection table entries supported on this IOAPIC. */
	ReadFromIoApic(ioapic_base_va, IOAPIC_REGISTER_IOAPIC_VERSION, &ver.dword );
	/*! loop the the table entries and add their starting vector number*/
	for(i=0; i<= ver.max_redirection_entries; i++)
	{
		/*! read the redirection table entry, modify it and write back*/
		ReadFromIoApic( ioapic_base_va, table_index, &redirect_table.dword_low );
		redirect_table.interrupt_vector = start_vector+i;
		redirect_table.interrupt_mask = 0;
		/*! \todo add destination field - to interrupt only master cpu*/
		WriteToIoApic( ioapic_base_va, table_index, redirect_table.dword_low );
		
		table_index+=2;
	}
	return i;
}
void MaskIoApicInterrupt(IOAPIC_REGISTER_SELECTOR_PTR ioapic_base_va, BYTE vector)
{
	IOAPIC_REDIRECT_TABLE redirect_table;
	int table_index = IOAPIC_REGISTER_REDIRECT_TABLE + (2*vector);
	
	/*! read the redirection table entry, modify it and write back*/
	ReadFromIoApic( ioapic_base_va, table_index, &redirect_table.dword_low );
	redirect_table.interrupt_mask = 1;
	WriteToIoApic( ioapic_base_va, table_index, redirect_table.dword_low );
}

void MaskInterrupt(BYTE interrupt_number)
{
	int i;
	for(i=0; i<count_ioapic; i++)
	{
		if ( (interrupt_number >= ioapic[i].start_irq) && (interrupt_number <= ioapic[i].end_irq) )
		{
			MaskIoApicInterrupt( ioapic[i].base_virtual_address, interrupt_number);
			break;
		}
	}
}
