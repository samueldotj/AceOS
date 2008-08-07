/*!
  \file		kernel/ioapic.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Wed Aug 06, 2008  11:25PM
  			Last modified: Thu Aug 07, 2008  03:37PM
  \brief	Contains IO-APIC stuff.
*/


#ifndef _IOAPIC_H_
#define _IOAPIC_H_

#include <ace.h>

#define MAX_IOAPIC 4
#define IOAPIC_STARTING_VECTOR_NUMBER 32
#define IOAPIC_BASE_MSR_START 0xfec00000

//IOAPIC registers


/* IOAPIC */
typedef struct ioapic
{
	UINT32 ioapic_id;
	UINT32 physical_address;
	UINT32 starting_vector;
	UINT32 max_entries;
}IOAPIC, *IOAPIC_PTR;

enum IOAPIC_REGISTER
{
	IOAPIC_REGISTER_IOAPIC_ID=0,
	IOAPIC_REGISTER_IOAPIC_VERSION=1,
	IOAPIC_REGISTER_IOAPIC_ARBITRATION=2,
	IOAPIC_REGISTER_REDIRECT_TABLE0=0x10,
	IOAPIC_REGISTER_REDIRECT_TABLE1=0x12,
	IOAPIC_REGISTER_REDIRECT_TABLE2=0x14,
	IOAPIC_REGISTER_REDIRECT_TABLE3=0x16,
	IOAPIC_REGISTER_REDIRECT_TABLE4=0x18,
	IOAPIC_REGISTER_REDIRECT_TABLE5=0x1A,
	IOAPIC_REGISTER_REDIRECT_TABLE6=0x1C,
	IOAPIC_REGISTER_REDIRECT_TABLE7=0x1E,
	IOAPIC_REGISTER_REDIRECT_TABLE8=0x20,
	IOAPIC_REGISTER_REDIRECT_TABLE9=0x22,
	IOAPIC_REGISTER_REDIRECT_TABLE10=0x24,
	IOAPIC_REGISTER_REDIRECT_TABLE11=0x26,
	IOAPIC_REGISTER_REDIRECT_TABLE12=0x28,
	IOAPIC_REGISTER_REDIRECT_TABLE13=0x2A,
	IOAPIC_REGISTER_REDIRECT_TABLE14=0x2C,
	IOAPIC_REGISTER_REDIRECT_TABLE15=0x2E,
	IOAPIC_REGISTER_REDIRECT_TABLE16=0x30,
	IOAPIC_REGISTER_REDIRECT_TABLE17=0x32,
	IOAPIC_REGISTER_REDIRECT_TABLE18=0x34,
	IOAPIC_REGISTER_REDIRECT_TABLE19=0x36,
	IOAPIC_REGISTER_REDIRECT_TABLE20=0x38,
	IOAPIC_REGISTER_REDIRECT_TABLE21=0x3A,
	IOAPIC_REGISTER_REDIRECT_TABLE22=0x3C,
	IOAPIC_REGISTER_REDIRECT_TABLE23=0x3E
};


/* This structure is reimposed with IA32_APIC_BASE_MSR structure. So be careful in handling the reserved fields. */
typedef struct ioapic_reg
{
	//First byte specifies the register which has to be accessed.
	UINT32 reg:8,	//IO_REG_SEL register
		   reserved1:24;
	UINT32 reserved2[3];

	//at an offset of 4 bytes from base, specify the data.
	UINT32 data;	//IO_WIN register. This must be accessed as an Dword only.
}IOAPIC_REG, *IOAPIC_REG_PTR;

typedef struct ioapic_id
{
	UINT32 reserved1		:	24, //0-23
		   ioapic_id		:	4, //24-27
		   reserved2		:	4; //28-31
}IOAPIC_ID, *IOAPIC_ID_PTR;

typedef struct ioapic_version
{
	UINT32	apic_version			:	8, //0-7
			reserved1				:	8, //8-15
			max_redirection_entries	:	8, //16-23
			reserved2				:	8; //24-31
}IOAPIC_VERSION, *IOAPIC_VERSION_PTR;

typedef struct ioapic_arbitration
{
	UINT32	reserved1		:	24, //0-23
			arbitration_id	:	4, //24-27
			reserved2		:	4; //28-31
}IOAPIC_ARBITRATION, *IOAPIC_ARBITRATION_PTR;

typedef struct ioapic_redirect_table
{
	UINT32	interrupt_vector	:	8, //0-7
			delivery_mode		:	3, //8-10
			destination_mode	:	1, //1
			delivery_status		:	1, //12
			input_polarity		:	1, //13
			remote_irr			:	1, //14
			trigger_mode		:	1, //15
			interrupt_mask		:	1, //16
			reserved1			:	15; //17-31

	UINT32	reserved2			:	24, //31-55
			destination_field	:	8; //56-63
}IOAPIC_REDIRECT_TABLE, *IOAPIC_REDIRECT_TABLE_PTR;

enum IOAPIC_TRIGGER_MODE
{
	IOAPIC_TRIGGER_MODE_EDGE, //0
	IOAPIC_TRIGGER_MODE_LEVEL //1
};

enum IOAPIC_INPUT_POLARITY
{
	IOAPIC_INPUT_POLARITY_HIGH_ACTIVE, 	//0
	IOAPIC_INPUT_POLARITY_LOW_ACTIVE 	//1
};

enum IOAPIC_DELIVERY_STATUS
{
	IOAPIC_DELIVERY_STATUS_IDLE, 		//0
	IOAPIC_DELIVERY_STATUS_SEND_PENDING //1
};

enum IOAPIC_DESTINATION_MODE
{
	IOAPIC_DESTINATION_MODE_PHYSICAL,	//0
	IOAPIC_DESTINATION_MODE_LOGICAL 	//1
};

enum IOAPIC_DELIVERY_MODE
{
	IOAPIC_DELIVERY_MODE_FIXED, 			//0
	IOAPIC_DELIVERY_MODE_LOWEST_PRIORITY,	//1
	IOAPIC_DELIVERY_MODE_SMI,				//2
	IOAPIC_DELIVERY_MODE_RESERVED1,			//3
	IOAPIC_DELIVERY_MODE_NMI,				//4
	IOAPIC_DELIVERY_MODE_INIT,				//5
	IOAPIC_DELIVERY_MODE_RESERVED2,			//6
	IOAPIC_DELIVERY_MODE_ExtINT				//7
};

extern IOAPIC ioapic[MAX_IOAPIC];
extern UINT8 count_ioapic;

void RelocateBaseIOAPICAddress(UINT32 addr, UINT32 index);
void InitIOAPIC(void);
void ReadFromIOAPIC(enum IOAPIC_REGISTER reg, UINT32 *data, UINT8 index);
void WriteToIOAPIC(enum IOAPIC_REGISTER reg, UINT32 data, UINT8 index);
UINT8 GetIOAPICId(UINT8 index);
void SetIOAPICId(UINT8 ioapic_id, UINT8 index);
UINT8 GetMaximumIOAPICRedirectionEntries(UINT8 index);
void GetIOAPICRedirectionTableEntry(enum IOAPIC_REGISTER reg, IOAPIC_REDIRECT_TABLE_PTR table, UINT8 index);
void SetIOAPICRedirectionTableEntry(enum IOAPIC_REGISTER reg, IOAPIC_REDIRECT_TABLE_PTR table, UINT8 index);
int InitIOAPICRedirectionTable(int starting_vector, UINT8 index);

#endif
