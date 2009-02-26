/*!
  \file		kernel/i386/apic.h
  \brief	Advanced Programmable Interrupt Controller
*/

#ifndef _APIC_H_
#define _APIC_H_

#include <ace.h>

/*! Default base address of the LAPIC*/
#define LAPIC_BASE_MSR_START 		0xFEE00000
#define LAPIC_MEMORY_MAP_SIZE		4096

/*! returns true if the current processor is primary processor*/
#define AM_I_BOOTSTRAP_PROCESSOR 	ia32_lapic_base_msr.bsp

/* Enums */
typedef enum
{
    ICR_DELIVERY_MODE_FIXED,
    ICR_DELIVERY_MODE_LOWEST_PRIORITY,
    ICR_DELIVERY_MODE_SMI,
    ICR_DELIVERY_MODE_RESERVED,
    ICR_DELIVERY_MODE_NMI,
    ICR_DELIVERY_MODE_INIT,
    ICR_DELIVERY_MODE_SIPI,
    ICR_DELIVERY_MODE_ExtINT
}ICR_DELIVERY_MODE;

typedef enum
{
    ICR_DESTINATION_MODE_PHYSICAL,
    ICR_DESTINATION_MODE_LOGICAL
}ICR_DESTINATION_MODE;

typedef enum
{
    ICR_DELIVERY_STATUS_IDLE,
    ICR_DELIVERY_STATUS_SEND_PENDING
}ICR_DELIVERY_STATUS;

typedef enum
{
    ICR_LEVEL_DE_ASSERT,
    ICR_LEVEL_ASSERT
}ICR_LEVEL;

typedef enum
{
    ICR_TRIGGER_MODE_EDGE,
    ICR_TRIGGER_MODE_LEVEL
}ICR_TRIGGER_MODE;

typedef enum
{
    ICR_DESTINATION_SHORTHAND_NO_SHORTHAND,
    ICR_DESTINATION_SHORTHAND_SELF,
    ICR_DESTINATION_SHORTHAND_ALL_INCLUDING_SELF,
    ICR_DESTINATION_SHORTHAND_ALL_EXCLUDING_SELF
}ICR_DESTINATION_SHORTHAND;

typedef enum
{
    DFR_MODEL_FLAT_MODEL,
    DFR_MODEL_CLUSTER_MODEL
}DFR_MODEL;

/*! LAPIC */
typedef struct ia32_apic_base_msr
{
	union
	{
		struct 
		{
			UINT32  base_high: 4,
					reserved3: 28;
		};
		UINT32 dword_low;
	};
	union
	{
		struct 
		{
			UINT32  reserved1: 8,
					bsp: 1,
					reserved2: 2,
					enable: 1,
					base_low: 20;
		};
		UINT32	dword_high;
	};
} IA32_APIC_BASE_MSR, * IA32_APIC_BASE_MSR_PTR;

typedef union interrupt_command_register_low
{
	struct 
	{
		UINT32  vector: 8,
				delivery_mode: 3,
				destination_mode: 1,
				delivery_status: 1,
				reserved1: 1,
				level: 1,
				trigger_mode: 1,
				reserved2: 2,
				destination_shorthand: 2,
				reserved3: 12;
	};
	UINT32 dword;
}INTERRUPT_COMMAND_REGISTER_LOW, * INTERRUPT_COMMAND_REGISTER_LOW_PTR;

typedef union interrupt_command_register_high
{
	struct
	{
		UINT32  reserved: 24,
				destination_field: 8;
	};
	UINT32 dword;
}INTERRUPT_COMMAND_REGISTER_HIGH, * INTERRUPT_COMMAND_REGISTER_HIGH_PTR;

/*! Local Vector Table */
typedef union lvt_timer_register
{
	struct
	{
		UINT32  vector: 8,
				reserved1: 4,
				delivery_status: 1,
				reserved2: 3,
				mask: 1,
				timer_periodic_mode: 1,
				reserved3: 14;
	};
	UINT32 dword;
}LVT_TIMER_REGISTER, * LVT_TIMER_REGISTER_PTR;

typedef union lvt_lint0_reg
{
	struct
	{
		UINT32  vector: 8,
				delivery_mode: 3,
				reserved1: 1,
				delivery_status: 1,
				interrupt_input_pin_polarity: 1,
				remote_irr: 1,
				trigger_mode: 1,
				mask: 1,
				reserved2: 15;
	};
	UINT32 dword;
}LVT_LINT0_REG, * LVT_LINT0_REG_PTR;

typedef union lvt_lint1_reg
{
	struct
	{
		UINT32  vector: 8,
				delivery_mode: 3,
				reserved1: 1,
				delivery_status: 1,
				interrupt_input_pin_polarity: 1,
				remote_irr: 1,
				trigger_mode: 1,
				mask: 1,
				reserved2: 15;
	};
	UINT32 dword;
}LVT_LINT1_REG, * LVT_LINT1_REG_PTR;

typedef union lvt_error_reg
{
	struct 
	{
		UINT32  vector: 8,
				reserved1: 4,
				delivery_status: 1,
				reserved2: 3,
				mask: 1,
				reserved3: 15;
	};
	UINT32 dword;
}LVT_ERROR_REG, * LVT_ERROR_REG_PTR;

typedef union lvt_performance_monitor_count_reg
{
	struct 
	{
		UINT32  vector: 8,
				delivery_mode: 3,
				reserved1: 1,
				delivery_status: 1,
				reserved2: 3,
				mask: 1,
				reserved3: 15;
	};
	UINT32 dword;
}LVT_PERFORMANCE_MONITOR_COUNT_REG, * LVT_PERFORMANCE_MONITOR_COUNT_REG_PTR;

typedef union lvt_thermal_sensor_reg
{
	struct
	{
		UINT32  vector: 8,
				delivery_mode: 3,
				reserved1: 1,
				delivery_status: 1,
				reserved2: 3,
				mask: 1,
				reserved3: 15;
	};
	UINT32 dword;
}LVT_THERMAL_SENSOR_REG, * LVT_THERMAL_SENSOR_REG_PTR;

typedef union error_status_reg
{
	struct
	{
		UINT32  send_checksum_error: 1,
				receive_checksum_error: 1,
				send_accept_error: 1,
				receive_accept_error: 1,
				reserved1: 1,
				send_illegal_vector: 1,
				received_illegal_vector: 1,
				illegal_register_address: 1,
				reserved2: 24; 
	};
	UINT32 dword;
}ERROR_STATUS_REG, * ERROR_STATUS_REG_PTR;

typedef union local_apic_version_reg
{
	struct
	{
		UINT32  version: 8,
				reserved1: 8,
				max_lvt_entry: 8,
				reserved2: 8;
	};
	UINT32 dword;
}LOCAL_APIC_VERSION_REG, * LOCAL_APIC_VERSION_REG_PTR;

typedef union logical_destination_reg
{
	struct
	{
		UINT32  reserved1: 24,
				logical_apic_id: 8;
	};
	UINT32 dword;
}LOGICAL_DESTINATION_REG, * LOGICAL_DESTINATION_REG_PTR;

typedef union destination_format_reg
{
	struct
	{
		UINT32  reserved1: 28,
				model: 4;
	};
	UINT32 dword;
}DESTINATION_FORMAT_REG, * DESTINATION_FORMAT_REG_PTR;

typedef union arbitration_priority_reg
{
	struct
	{
		UINT32  arbitration_priority_subclass: 4,
				arbitration_priority:4,
				reserved: 24;
	};
	UINT32 dword;
}ARBITRATION_PRIORITY_REG, * ARBITRATION_PRIORITY_REG_PTR;

typedef union task_priority_reg
{
	struct
	{
		UINT32  task_priority_subclass: 4,
				task_priority: 4,
				reserved: 24;
	};
	UINT32 dword;
}TASK_PRIORITY_REG, *TASK_PRIORITY_REG_PTR;

typedef union processor_priority_reg
{
	struct
	{
		UINT32  processor_priority_subclass: 4,
				processor_priority: 4,
				reserved: 24;
	};
	UINT32 dword;
}PROCESSOR_PRIORITY_REG, *PROCESSOR_PRIORITY_REG_PTR;

typedef union interrupt_request_reg
{
    UINT32  vectors[8];
    UINT16  reserved: 16;
}INTERRUPT_REQUEST_REG, * INTERRUPT_REQUEST_REG_PTR;

typedef union in_service_reg
{
    UINT16 reserved: 16;
    UINT32 vectors[8];
}IN_SERVICE_REG, * IN_SERVICE_REG_PTR;

typedef union trigger_mode_reg
{
    UINT16 reserved: 16;
    UINT32 vectors[8];
}TRIGGER_MODE_REG, * TRIGGER_MODE_REG_PTR;

typedef struct eoi_reg
{
    UINT32 zero;
}EOI_REG, * EOI_REG_PTR;

typedef union spurious_interrrupt_reg
{
	struct
	{
		UINT32	spurious_vector:8,
				apic_enable:1,
				focus_processor_checking:1,
				reserved:22;
	};
	UINT32 dword;
}SPURIOUS_INTERRUPT_REG, * SPURIOUS_INTERRUPT_REG_PTR;

extern IA32_APIC_BASE_MSR_PTR lapic_base_address;

void InitLAPIC(void);
void SendEndOfInterruptToLapic(int int_no);

void IssueInterprocessorInterrupt(BYTE vector, UINT32 apic_id, ICR_DELIVERY_MODE delivery_mode, ICR_DESTINATION_SHORTHAND destination_shorthand);
int StartProcessor(UINT32 apic_id, UINT32 physical_address);

UINT32 GetInterruptPriorityLevel(void);
UINT32 SetInterruptPriorityLevel(UINT32 ipl);

#endif
