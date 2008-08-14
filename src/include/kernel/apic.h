/*!
  \file		kernel/apic.h
  \brief	Advanced Programmable Interrupt Controller
*/

#ifndef _APIC_H_
#define _APIC_H_

#include <ace.h>

/*! Default base address of the LAPIC*/
#define LAPIC_BASE_MSR_START 		0xfee00000
/*! returns true if the current processor is primary processor*/
#define AM_I_BOOTSTRAP_PROCESSOR 	ia32_lapic_base_msr.bsp

/* Enums */
enum ICR_DELIVERY_MODE
{
    ICR_DELIVERY_MODE_FIXED,
    ICR_DELIVERY_MODE_LOWEST_PRIORITY,
    ICR_DELIVERY_MODE_SMI,
    ICR_DELIVERY_MODE_RESERVED1,
    ICR_DELIVERY_MODE_NMI,
    ICR_DELIVERY_MODE_INIT,
    ICR_DELIVERY_MODE_SIPI,
    ICR_DELIVERY_MODE_ExtINT
};

enum ICR_DESTINATION_MODE
{
    ICR_DESTINATION_MODE_PHYSICAL,
    ICR_DESTINATION_MODE_LOGICAL
};

enum ICR_DELIVERY_STATUS
{
    ICR_DELIVERY_STATUS_IDLE,
    ICR_DELIVERY_STATUS_SEND_PENDING
};

enum ICR_LEVEL
{
    ICR_LEVEL_DE_ASSERT,
    ICR_LEVEL_ASSERT
};

enum ICR_TRIGGER_MODE
{
    ICR_TRIGGER_MODE_EDGE,
    ICR_TRIGGER_MODE_LEVEL
};

enum ICR_DESTINATION_SHORTHAND
{
    ICR_DESTINATION_SHORTHAND_NO_SHORTHAND,
    ICR_DESTINATION_SHORTHAND_SELF,
    ICR_DESTINATION_SHORTHAND_ALL_INCLUDING_SELF,
    ICR_DESTINATION_SHORTHAND_ALL_EXCLUDING_SELF
};

enum DFR_MODEL
{
    DFR_MODEL_FLAT_MODEL,
    DFR_MODEL_CLUSTER_MODEL
};

/* LAPIC */
typedef struct ia32_apic_base_msr
{
    UINT32  reserved1: 8,
            bsp: 1,
            reserved2: 2,
            enable: 1,
            base_low: 20;
    UINT32  base_high: 4,
            reserved3: 28;
} IA32_APIC_BASE_MSR, * IA32_APIC_BASE_MSR_PTR;

typedef struct interrupt_command_register
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
    UINT32  reserved4: 24,
            destination_field: 8;
}INTERRUPT_COMMAND_REGISTER, * INTERRUPT_COMMAND_REGISTER_PTR;

/* Start of Local Vector Table */
typedef struct timer_register
{
    UINT32  vector: 8,
            reserved1: 4,
            delivery_status: 1,
            reserved2: 3,
            mask: 1,
            timer_mode: 1,
            reserved3: 14;
}TIMER_REGISTER, * TIMER_REGISTER_PTR;

typedef struct lint0_reg
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
}LINT0_REG, * LINT0_REG_PTR;

typedef struct lint1_reg
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
}LINT1_REG, * LINT1_REG_PTR;

typedef struct error_reg
{
    UINT32  vector: 8,
            reserved1: 4,
            delivery_status: 1,
            reserved2: 3,
            mask: 1,
            reserved3: 15;
}ERROR_REG, * ERROR_REG_PTR;

typedef struct performance_monitor_count_reg
{
    UINT32  vector: 8,
            delivery_mode: 3,
            reserved1: 1,
            delivery_status: 1,
            reserved2: 3,
            mask: 1,
            reserved3: 15;
}PERFORMANCE_MONITOR_COUNT_REG, * PERFORMANCE_MONITOR_COUNT_REG_PTR;

typedef struct thermal_sensor_reg
{
    UINT32  vector: 8,
            delivery_mode: 3,
            reserved1: 1,
            delivery_status: 1,
            reserved2: 3,
            mask: 1,
            reserved3: 15;
}THERMAL_SENSOR_REG, * THERMAL_SENSOR_REG_PTR;

typedef struct error_status_reg
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
}ERROR_STATUS_REG, * ERROR_STATUS_REG_PTR;

typedef struct local_apic_version_reg
{
    UINT32  version: 8,
            reserved1: 8,
            max_lvt_entry: 8,
            reserved2: 8;
}LOCAL_APIC_VERSION_REG, * LOCAL_APIC_VERSION_REG_PTR;

typedef struct logical_destination_reg
{
    UINT32  reserved1: 24,
            logical_apic_id: 8;
}LOGICAL_DESTINATION_REG, * LOGICAL_DESTINATION_REG_PTR;

typedef struct destination_format_reg
{
    UINT32  reserved1: 28,
            model: 4;
}DESTINATION_FORMAT_REG, * DESTINATION_FORMAT_REG_PTR;

typedef struct arbitration_priority_reg
{
    UINT32  arbitration_priority_subclass: 4,
            arbitration_priority:4,
            reserved: 24;
}ARBITRATION_PRIORITY_REG, * ARBITRATION_PRIORITY_REG_PTR;

typedef struct task_priority_reg
{
    UINT32  task_priority_subclass: 4,
            task_priority: 4,
            reserved: 24;
}TASK_PRIORITY_REG, *TASK_PRIORITY_REG_PTR;

typedef struct processor_priority_reg
{
    UINT32  processor_priority_subclass: 4,
            processor_priority: 4,
            reserved: 24;
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

typedef struct spurious_interrrupt_reg
{
	UINT32	spurious_vector:8,
			apic_enable:1,
			focus_processor_checking:1,
			reserved;
}SPURIOUS_INTERRUPT_REG, * SPURIOUS_INTERRUPT_REG_PTR;

extern IA32_APIC_BASE_MSR_PTR lapic_base_msr;

int DetectAPIC(UINT8 cpu_id);
void UseAPIC(int enable);
void InitAPIC(void);
void SendEndOfInterrupt(int int_no);
void RelocateBaseLAPICAddress(UINT32 addr);
void InitSmp(void);
INT16 IssueInterprocessorInterrupt(UINT32 vector, UINT32 apic_id, enum ICR_DELIVERY_MODE delivery_mode,
                enum ICR_DESTINATION_SHORTHAND destination_shorthand, BYTE init_de_assert);

#endif
