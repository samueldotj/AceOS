/*!
  \file		ioapic.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:
  			Last modified: Wed Aug 06, 2008  11:58PM
  \brief	Contains APIC stuff in general and LAPIC.
*/

#include <ace.h>
#include <kernel/ioapic.h>
#include <kernel/apic.h>


IOAPIC_REG_PTR ioapic_reg;

/*!
	\brief	 Initializies IOAPIC memory mapped registers and redirection table.

	\param	 void

	\return	 void
*/
void InitIOAPIC(void)
{
	//First initialise our 2 memory mapped registers.
	ioapic_reg = (IOAPIC_REG_PTR)(ia32_ioapic_base_msr);
	//Now setup the redirection table.
	InitIOAPICRedirectionTable(IOAPIC_STARTING_VECTOR_NUMBER);
	return;
}


/*!
	\brief	Read data from IOAPIC for the given register.

	\param	reg: IOAPIC register to be accessed.
			data: Pointer to 32 bit memory in which data is filled from IOAPIC.

	\return	 void
*/
void ReadFromIOAPIC(enum IOAPIC_REGISTER reg, UINT32 *data)
{
	ioapic_reg->reg = reg;
	*data = ioapic_reg->data;
}


/*!
	\brief	Write to the specified register in IOAPIC.

	\param	reg: IOAPIC register to be accessed.
			data: 32 bit data that has to be written into the specified register.

	\return	 void
*/
void WriteToIOAPIC(enum IOAPIC_REGISTER reg, UINT32 data)
{
	ioapic_reg->reg = reg;
	ioapic_reg->data = data;
}


/*!
	\brief	Fetch the IOAPIC ID. 

	\param	 void

	\return	 4 bit IOAPIC id.
*/
UINT8 GetIOAPICId(void)
{
	UINT32 data;
	ioapic_reg->reg = IOAPIC_REGISTER_IOAPIC_ID;
	data = ioapic_reg->data;
	return (UINT8)( ((IOAPIC_ID_PTR)data)->ioapic_id );
}

/*!
	\brief	Set the IOAPIC ID. 

	\param	ioapic_id: unique id for IOAPIC which is to be set.

	\return	 void
*/
void SetIOAPICId(UINT8 ioapic_id)
{
	UINT32 data;
   	((IOAPIC_ID_PTR)&data)->ioapic_id	= ioapic_id;
	ioapic_reg->reg = IOAPIC_REGISTER_IOAPIC_ID;
	ioapic_reg->data = data;
	return;
}


/*!
	\brief	 Get the maximum number of entries in IOAPIC redirection table. These many Interrupt lines are avilable.

	\param	 void

	\return	 Positive number of redirection entries.
*/
UINT8 GetMaximumIOAPICRedirectionEntries(void)
{
	UINT32 data;
	ioapic_reg->reg = IOAPIC_REGISTER_IOAPIC_VERSION;
	data = ioapic_reg->data;
	return (UINT8)( ((IOAPIC_VERSION_PTR)&data)->max_redirection_entries );
}



/*!
	\brief	Load the redirection table structure with details obtained from IOAPIC for the required vector.

	\param	reg:	IOAPIC register that is to be accessed.
			table:	Pointer to redirection table which has to be loaded with details from IOAPIC.

	\return	void
*/
void GetIOAPICRedirectionTableEntry(enum IOAPIC_REGISTER reg, IOAPIC_REDIRECT_TABLE_PTR table)
{
	UINT32 data;
	ioapic_reg->reg = reg;
	data = ioapic_reg->data;
	
	*table = *((IOAPIC_REDIRECT_TABLE_PTR)(&data));

	ioapic_reg->reg = reg + 0x1;
	data = ioapic_reg->data;

	table->destination_field = (data & 0xff000000); //get the MSB 8 bits
	return;
}


/*!
	\brief	Set the IOAPIC redirection table with given details.

	\param	reg:	IOAPIC register that is to be accessed.
			table:	Pointer to redirection table which has to be loaded with details from IOAPIC.

	\return	void
*/
void SetIOAPICRedirectionTableEntry(enum IOAPIC_REGISTER reg, IOAPIC_REDIRECT_TABLE_PTR table)
{
	UINT32 data = (UINT32)((char*)(table));
	ioapic_reg->reg = reg;
	ioapic_reg->data = data;
	
	data = (UINT32)( (table->destination_field) << 24 );

	ioapic_reg->reg = reg + 0x1;
	ioapic_reg->data = data;

	return;
}


/*!
	\brief	Initial set up the redirection table entry to deliver Interrupts as vector number starting from <starting_vector>

	\param	starting_vector: The starting vector number from which interrupts have to be redirected.

	\return	0: Success
			-1: Invalid starting vector number.
			-2: Invalid max entries in redirection table.
*/
int InitIOAPICRedirectionTable(int starting_vector)
{
	UINT8 max_entries, i;
	IOAPIC_REDIRECT_TABLE redirect_table;
	enum IOAPIC_REGISTER reg;

	if (starting_vector < 16 || starting_vector > 230) //254-24 = 230
		return -1;

	//Get the number of maximum redirection table entries supported on this IOAPIC.
	max_entries = GetMaximumIOAPICRedirectionEntries();
	if( max_entries > 24)
		return -2;

	for(i=0; i< max_entries; i++)
	{
		redirect_table.interrupt_vector = starting_vector + i;
		reg = (enum IOAPIC_REGISTER)(0x10 + i*2);
		SetIOAPICRedirectionTableEntry(reg, &redirect_table);
	}
	return 0;
}
