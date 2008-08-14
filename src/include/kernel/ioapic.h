/*!
  \file		kernel/ioapic.h
  \brief	Contains IO-APIC stuff.
*/


#ifndef _IOAPIC_H_
#define _IOAPIC_H_

#include <ace.h>

/*! Maximum IOAPIC supported by Ace*/
#define MAX_IOAPIC 16
/*! Starting interrrupt vector number for IOAPIC*/
#define IOAPIC_STARTING_VECTOR_NUMBER 32
/*!	Default base address of IOAPIC*/
#define IOAPIC_BASE_MSR_START 0xfec00000

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
	UINT32 reserved1		:	24,
		   ioapic_id		:	4,
		   reserved2		:	4;
}IOAPIC_ID, *IOAPIC_ID_PTR;

typedef struct ioapic_version
{
	UINT32	apic_version			:	8,
			reserved1				:	8,
			max_redirection_entries	:	8,
			reserved2				:	8;
}IOAPIC_VERSION, *IOAPIC_VERSION_PTR;

typedef struct ioapic_arbitration
{
	UINT32	reserved1		:	24,
			arbitration_id	:	4,
			reserved2		:	4;
}IOAPIC_ARBITRATION, *IOAPIC_ARBITRATION_PTR;

typedef struct ioapic_redirect_table
{
	UINT32	interrupt_vector	:	8,
			delivery_mode		:	3,
			destination_mode	:	1,
			delivery_status		:	1,
			input_polarity		:	1,
			remote_irr			:	1,
			trigger_mode		:	1,
			interrupt_mask		:	1,
			reserved1			:	15;

	UINT32	reserved2			:	24,
			destination_field	:	8;
}IOAPIC_REDIRECT_TABLE, *IOAPIC_REDIRECT_TABLE_PTR;

enum IOAPIC_TRIGGER_MODE
{
	IOAPIC_TRIGGER_MODE_EDGE,
	IOAPIC_TRIGGER_MODE_LEVEL
};

enum IOAPIC_INPUT_POLARITY
{
	IOAPIC_INPUT_POLARITY_HIGH_ACTIVE,
	IOAPIC_INPUT_POLARITY_LOW_ACTIVE
};

enum IOAPIC_DELIVERY_STATUS
{
	IOAPIC_DELIVERY_STATUS_IDLE,
	IOAPIC_DELIVERY_STATUS_SEND_PENDING
};

enum IOAPIC_DESTINATION_MODE
{
	IOAPIC_DESTINATION_MODE_PHYSICAL,
	IOAPIC_DESTINATION_MODE_LOGICAL
};

enum IOAPIC_DELIVERY_MODE
{
	IOAPIC_DELIVERY_MODE_FIXED,
	IOAPIC_DELIVERY_MODE_LOWEST_PRIORITY,
	IOAPIC_DELIVERY_MODE_SMI,
	IOAPIC_DELIVERY_MODE_RESERVED1,
	IOAPIC_DELIVERY_MODE_NMI,
	IOAPIC_DELIVERY_MODE_INIT,
	IOAPIC_DELIVERY_MODE_RESERVED2,
	IOAPIC_DELIVERY_MODE_ExtINT
};

extern IOAPIC ioapic[MAX_IOAPIC];
extern UINT8 count_ioapic;

void InitIOAPIC(void);
UINT8 GetIOAPICId(UINT8 index);
UINT8 GetMaximumIOAPICRedirectionEntries(UINT8 index);
void GetIOAPICRedirectionTableEntry(enum IOAPIC_REGISTER reg, IOAPIC_REDIRECT_TABLE_PTR table, UINT8 index);
void SetIOAPICRedirectionTableEntry(enum IOAPIC_REGISTER reg, IOAPIC_REDIRECT_TABLE_PTR table, UINT8 index);
int InitIOAPICRedirectionTable(int starting_vector, UINT8 index);

#endif
