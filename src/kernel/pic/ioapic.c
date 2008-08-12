/*!
  \file		ioapic.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:
  			Last modified: Tue Aug 12, 2008  09:31AM
  \brief	Contains APIC stuff in general and LAPIC.
*/

#include <ace.h>
#include <kernel/ioapic.h>
#include <kernel/apic.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/error.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/debug.h>
#include <string.h>


IOAPIC_REG_PTR ioapic_base_reg[MAX_IOAPIC]; //This can also be used as IA32_APIC_BASE_MSR_PTR

IOAPIC ioapic[MAX_IOAPIC];
UINT8 count_ioapic;

static void ReadFromIOAPIC(enum IOAPIC_REGISTER reg, UINT32 *data, UINT8 index);
static void WriteToIOAPIC(enum IOAPIC_REGISTER reg, UINT32 data, UINT8 index);
static int FindIOAPIC(UINT8 ioapic_id);


/*!
	\brief	 Search for IOAPIC using ioapic id as the primary key.

	\param	ioapic_id: unique id associated with each ioapic.

	\return	index of ioapic(Positive number): success
			-1: Failure
*/
static int FindIOAPIC(UINT8 ioapic_id)
{
	UINT32	index;

	for(index = 0; index < count_ioapic; index++)
	{
		if( ioapic_id ==  GetIOAPICId(index)) //found
			return index;
	}

	return -1;
}


/*!
	\brief	 Initializies IOAPIC memory mapped registers and redirection table.

	\param	 void

	\return	 void
*/
void InitIOAPIC(void)
{
	UINT32 index;
	
	for (index=0; index < count_ioapic; index++)
	{
		ioapic_base_reg[index] = (IOAPIC_REG_PTR)MapPhysicalMemory(&kernel_map, (ioapic[index]).physical_address, PAGE_SIZE);
		if(!ioapic_base_reg[index])
			panic("Mapping PA in ioapic failed\n");

		//Now setup the redirection table in each of the ioapic.
		/* Each IOAPIC is initialized from acpi and we load the GlobalIrqBase count in starting_vector.
		 * So use that info in getting the starting vector number for each of the apic.
		 */
		InitIOAPICRedirectionTable(IOAPIC_STARTING_VECTOR_NUMBER + (ioapic[index]).starting_vector, index);
	}

	return;
}


/*!
	\brief	Read data from IOAPIC for the given register.

	\param	reg: IOAPIC register to be accessed.
			data: Pointer to 32 bit memory in which data is filled from IOAPIC.

	\return	 void
*/
static void ReadFromIOAPIC(enum IOAPIC_REGISTER reg, UINT32 *data, UINT8 index)
{
	(ioapic_base_reg[index])->reg = reg;
	*data = (ioapic_base_reg[index])->data;
}


/*!
	\brief	Write to the specified register in IOAPIC.

	\param	reg: IOAPIC register to be accessed.
			data: 32 bit data that has to be written into the specified register.

	\return	 void
*/
static void WriteToIOAPIC(enum IOAPIC_REGISTER reg, UINT32 data, UINT8 index)
{
	(ioapic_base_reg[index])->reg = reg;
	(ioapic_base_reg[index])->data = data;
}


/*!
	\brief	Fetch the IOAPIC ID. 

	\param	index: index to the IOAPIC inside ioapic_base_reg array.

	\return	 4 bit IOAPIC id.
*/
UINT8 GetIOAPICId(UINT8 index)
{
	UINT32 data;
	(ioapic_base_reg[index])->reg = IOAPIC_REGISTER_IOAPIC_ID;
	data = (ioapic_base_reg[index])->data;
	return (UINT8)( ((IOAPIC_ID_PTR)data)->ioapic_id );
}

/*!
	\brief	 Get the maximum number of entries in IOAPIC redirection table. These many Interrupt lines are avilable.

	\param	 void

	\return	 Positive number of redirection entries.
*/
UINT8 GetMaximumIOAPICRedirectionEntries(UINT8 index)
{
	UINT32 data;
	(ioapic_base_reg[index])->reg = IOAPIC_REGISTER_IOAPIC_VERSION;
	data = (ioapic_base_reg[index])->data;
	return (UINT8)( ((IOAPIC_VERSION_PTR)&data)->max_redirection_entries );
}



/*!
	\brief	Load the redirection table structure with details obtained from IOAPIC for the required vector.

	\param	reg:	IOAPIC register that is to be accessed.
			table:	Pointer to redirection table which has to be loaded with details from IOAPIC.

	\return	void
*/
void GetIOAPICRedirectionTableEntry(enum IOAPIC_REGISTER reg, IOAPIC_REDIRECT_TABLE_PTR table, UINT8 index)
{
	UINT32 data;
	(ioapic_base_reg[index])->reg = reg;
	data = (ioapic_base_reg[index])->data;
	
	*table = *((IOAPIC_REDIRECT_TABLE_PTR)(&data));

	(ioapic_base_reg[index])->reg = reg + 0x1;
	data = (ioapic_base_reg[index])->data;

	table->destination_field = (data & 0xff000000); //get the MSB 8 bits
	return;
}


/*!
	\brief	Set the IOAPIC redirection table with given details.

	\param	reg:	IOAPIC register that is to be accessed.
			table:	Pointer to redirection table which has to be loaded with details from IOAPIC.

	\return	void
*/
void SetIOAPICRedirectionTableEntry(enum IOAPIC_REGISTER reg, IOAPIC_REDIRECT_TABLE_PTR table, UINT8 index)
{
	UINT32 data = (UINT32)((char*)(table));
	(ioapic_base_reg[index])->reg = reg;
	(ioapic_base_reg[index])->data = data;
	
	data = (UINT32)( (table->destination_field) << 24 );

	(ioapic_base_reg[index])->reg = reg + 0x1;
	(ioapic_base_reg[index])->data = data;

	return;
}


/*!
	\brief	Initial set up the redirection table entry to deliver Interrupts as vector number starting from <starting_vector>

	\param	starting_vector: The starting vector number from which interrupts have to be redirected.

	\return	0: Success
			-1: Invalid starting vector number.
			-2: Invalid max entries in redirection table.
*/
int InitIOAPICRedirectionTable(int starting_vector, UINT8 index)
{
	UINT8 max_entries, i;
	IOAPIC_REDIRECT_TABLE redirect_table;
	enum IOAPIC_REGISTER reg;

	if (starting_vector < 16 || starting_vector > 230) //254-24 = 230
		return -1;

	//Get the number of maximum redirection table entries supported on this IOAPIC.
	max_entries = GetMaximumIOAPICRedirectionEntries(index);
	if( max_entries > 24)
		return -2;

	ioapic[index].max_entries = max_entries;

	for(i=0; i< max_entries; i++)
	{
		redirect_table.interrupt_vector = starting_vector + i;
		reg = (enum IOAPIC_REGISTER)(0x10 + i*2);
		SetIOAPICRedirectionTableEntry(reg, &redirect_table, index);
	}
	return 0;
}
