/*!
  \file		kernel/ioapic.h
  \brief	Contains IO-APIC stuff.
*/


#ifndef _IOAPIC_H_
#define _IOAPIC_H_

#include <ace.h>

#ifdef CONFIG_SMP
	#define MAX_IOAPIC 16	/*! Maximum IOAPIC supported by Ace*/
#else
	#define MAX_IOAPIC 1
#endif

/*! Interrrupt vector numbers 
	0	-	31		Exceptions
	32	-	47		PIC IRQs
	32	-	xx		IOAPIC IRQs
	..
	238	-	244		APIC local interrupts
*/
#define PIC_STARTING_VECTOR_NUMBER 			32
#define IOAPIC_STARTING_VECTOR_NUMBER		PIC_STARTING_VECTOR_NUMBER

#define LOCAL_TIMER_VECTOR_NUMBER			238
#define SPURIOUS_VECTOR_NUMBER				(LOCAL_TIMER_VECTOR_NUMBER + 1)
#define LINT0_VECTOR_NUMBER					(LOCAL_TIMER_VECTOR_NUMBER + 2)
#define LINT1_VECTOR_NUMBER					(LOCAL_TIMER_VECTOR_NUMBER + 3)
#define ERROR_VECTOR_NUMBER					(LOCAL_TIMER_VECTOR_NUMBER + 4)
#define PERF_MON_VECTOR_NUMBER				(LOCAL_TIMER_VECTOR_NUMBER + 5)
#define THERMAL_SENSOR_VECTOR_NUMBER		(LOCAL_TIMER_VECTOR_NUMBER + 6)


/*! Assignment of IRQs in 8259*/
typedef enum
{
	LEGACY_DEVICE_IRQ_TIMER		=0,
	LEGACY_DEVICE_IRQ_KEYBOARD	=1,
	LEGACY_DEVICE_IRQ_CASCADE	=2,
	LEGACY_DEVICE_IRQ_COM2		=3,
	LEGACY_DEVICE_IRQ_COM1		=4,
	LEGACY_DEVICE_IRQ_PCI0		=5,
	LEGACY_DEVICE_IRQ_FLOPPY	=6,
	LEGACY_DEVICE_IRQ_PRINTER	=7,
	
	LEGACY_DEVICE_IRQ_RTC		=8,
	LEGACY_DEVICE_IRQ_PCI3		=9,
	LEGACY_DEVICE_IRQ_PCI2		=10,
	LEGACY_DEVICE_IRQ_PCI1		=11,
	LEGACY_DEVICE_IRQ_MOUSE		=12,
	LEGACY_DEVICE_IRQ_FPU		=13,
	LEGACY_DEVICE_IRQ_IDE0		=14,
	LEGACY_DEVICE_IRQ_IDE1		=15
}LEGACY_DEVICE_IRQ;

/*! This structure abstracts a IOAPIC, this is a software abstraction and IOAPIC has nothing todo in this*/
typedef struct ioapic
{
	UINT32	ioapic_id;					/*! IOAPIC id*/
	BYTE	start_irq;					/*! Starting global irq number*/
	BYTE	end_irq;					/*! End global irq number*/
		
	void *	base_physical_address;		/*! Base address where this IOAPIC can be accessed*/
	void * 	base_virtual_address;		/*! virtual address where it is mapped in the kernel address space*/
}IOAPIC, *IOAPIC_PTR;

/*! Enumerates the IOAPIC register index*/
typedef enum ioapic_register
{
	IOAPIC_REGISTER_IOAPIC_ID			=0,
	IOAPIC_REGISTER_IOAPIC_VERSION		=1,
	IOAPIC_REGISTER_IOAPIC_ARBITRATION	=2,
	IOAPIC_REGISTER_REDIRECT_TABLE		=0x10
}IOAPIC_REGISTER;

/*! IOAPIC IOREGSEL address*/
typedef union ioapic_register_selector
{
	struct
	{
		UINT32	ioapic_register:8,	
				reserved:24;
	};
	UINT32		dword;
}IOAPIC_REGISTER_SELECTOR, * IOAPIC_REGISTER_SELECTOR_PTR;

/*! IOAPIC ID data format*/
typedef union ioapic_id
{
	struct
	{
		UINT32 	reserved1	:24,
				ioapic_id	:4,
				reserved2	:4;
	};
	UINT32		dword;
}IOAPIC_ID, *IOAPIC_ID_PTR;

/*! IOAPIC version data format*/
typedef union ioapic_version
{
	struct
	{
		UINT32	apic_version			:8,
				reserved1				:8,
				max_redirection_entries	:8,
				reserved2				:8;
	};
	UINT32 		dword;
}IOAPIC_VERSION, *IOAPIC_VERSION_PTR;

/*! IOAPIC arbitration data format*/
typedef union ioapic_arbitration
{
	struct
	{
		UINT32	reserved1		:24,
				arbitration_id	:4,
				reserved2		:4;
	};
	UINT32		dword;
}IOAPIC_ARBITRATION, *IOAPIC_ARBITRATION_PTR;

/*! IOAPIC redirect table format*/
typedef struct ioapic_redirect_table
{
	union
	{
		struct
		{
			UINT32	reserved2			:24,
					destination_field	:8;
		};
		UINT32		dword_high;
	};
	union
	{
		struct
		{
			UINT32	interrupt_vector	:8,
					delivery_mode		:3,
					destination_mode	:1,
					delivery_status		:1,
					interrupt_polarity	:1,
					remote_irr			:1,
					trigger_mode		:1,
					interrupt_mask		:1,
					reserved1			:15;
		};
		UINT32		dword_low;
	};
}IOAPIC_REDIRECT_TABLE, *IOAPIC_REDIRECT_TABLE_PTR;

typedef enum 
{
	IOAPIC_TRIGGER_MODE_EDGE,
	IOAPIC_TRIGGER_MODE_LEVEL
}IOAPIC_TRIGGER_MODE;

typedef enum 
{
	IOAPIC_INPUT_POLARITY_HIGH_ACTIVE,
	IOAPIC_INPUT_POLARITY_LOW_ACTIVE
}IOAPIC_INPUT_POLARITY;

typedef enum
{
	IOAPIC_DELIVERY_STATUS_IDLE,
	IOAPIC_DELIVERY_STATUS_SEND_PENDING
}IOAPIC_DELIVERY_STATUS;

typedef enum
{
	IOAPIC_DESTINATION_MODE_PHYSICAL,
	IOAPIC_DESTINATION_MODE_LOGICAL
}IOAPIC_DESTINATION_MODE;

typedef enum
{
	IOAPIC_DELIVERY_MODE_FIXED,
	IOAPIC_DELIVERY_MODE_LOWEST_PRIORITY,
	IOAPIC_DELIVERY_MODE_SMI,
	IOAPIC_DELIVERY_MODE_RESERVED1,
	IOAPIC_DELIVERY_MODE_NMI,
	IOAPIC_DELIVERY_MODE_INIT,
	IOAPIC_DELIVERY_MODE_RESERVED2,
	IOAPIC_DELIVERY_MODE_ExtINT
}IOAPIC_DELIVERY_MODE;

extern UINT32 legacy_irq_redirection_table[16];
extern IOAPIC ioapic[MAX_IOAPIC];
extern UINT8 count_ioapic;

void InitPic(BYTE start_vector);
void MaskPic();
void SendEndOfInterruptTo8259(int int_no);

int InitIoApic(IOAPIC_REGISTER_SELECTOR_PTR ioapic_base_va, BYTE start_vector);

#endif
