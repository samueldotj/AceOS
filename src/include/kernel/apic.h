/*!
  \file		kernel/apic.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Sat Jun 14, 2008  06:19PM
  			Last modified: Tue Aug 12, 2008  01:13AM
  \brief	
*/


#ifndef _APIC_H_
#define _APIC_H_

#include <ace.h>

#define LAPIC_BASE_MSR_START 0xfee00000
#define AM_I_BOOTSTRAP_PROCESSOR ia32_lapic_base_msr.bsp


/* Enums */
enum ICR_DELIVERY_MODE
{
    ICR_DELIVERY_MODE_FIXED=0,
    ICR_DELIVERY_MODE_LOWEST_PRIORITY=1,
    ICR_DELIVERY_MODE_SMI=2,
    ICR_DELIVERY_MODE_RESERVED1=3,
    ICR_DELIVERY_MODE_NMI=4,
    ICR_DELIVERY_MODE_INIT=5,
    ICR_DELIVERY_MODE_SIPI=6, //Startup IPI
    ICR_DELIVERY_MODE_ExtINT=7
};

enum ICR_DESTINATION_MODE
{
    ICR_DESTINATION_MODE_PHYSICAL=0,
    ICR_DESTINATION_MODE_LOGICAL=1
};

enum ICR_DELIVERY_STATUS
{
    ICR_DELIVERY_STATUS_IDLE=0,
    ICR_DELIVERY_STATUS_SEND_PENDING=1
};

enum ICR_LEVEL
{
    ICR_LEVEL_DE_ASSERT=0,
    ICR_LEVEL_ASSERT=1
};

enum ICR_TRIGGER_MODE
{
    ICR_TRIGGER_MODE_EDGE=0,
    ICR_TRIGGER_MODE_LEVEL=1
};

enum ICR_DESTINATION_SHORTHAND
{
    ICR_DESTINATION_SHORTHAND_NO_SHORTHAND=0,
    ICR_DESTINATION_SHORTHAND_SELF=1,
    ICR_DESTINATION_SHORTHAND_ALL_INCLUDING_SELF=2,
    ICR_DESTINATION_SHORTHAND_ALL_EXCLUDING_SELF=3
};

enum DFR_MODEL
{
    DFR_MODEL_FLAT_MODEL=15,
    DFR_MODEL_CLUSTER_MODEL=0
};

/* Structures and unions */

/* LAPIC */
typedef struct ia32_apic_base_msr
{
    UINT32  reserved1: 8, //0-7
            bsp: 1, //8th bit-> Is this a bootstrap processor
            reserved2: 2, //9-10
            enable: 1, //11
            base_low: 20; //12-31
    UINT32  base_high: 4, //32-35
            reserved3: 28; //36-63
} IA32_APIC_BASE_MSR, * IA32_APIC_BASE_MSR_PTR;

typedef struct interrupt_command_register
{
    UINT32  vector: 8, //0-7
            delivery_mode: 3, //8-10
            destination_mode: 1, //11
            delivery_status: 1, //12
            reserved1: 1, //13
            level: 1, //14
            trigger_mode: 1, //15
            reserved2: 2, //16-17
            destination_shorthand: 2, //18-19
            reserved3: 12; //20-31
    UINT32  reserved4: 24, //32-55
            destination_field: 8; //56-63
}INTERRUPT_COMMAND_REGISTER, * INTERRUPT_COMMAND_REGISTER_PTR;

/* Start of Local Vector Table */
typedef struct timer_register
{
    UINT32  vector: 8, //0-7
            reserved1: 4, //8-11
            delivery_status: 1, //12
            reserved2: 3, //13-15
            mask: 1, //16
            timer_mode: 1, //17
            reserved3: 14; //18-31
}TIMER_REGISTER, * TIMER_REGISTER_PTR;

typedef struct lint0_reg
{
    UINT32  vector: 8, //0-7
            delivery_mode: 3, //8-10
            reserved1: 1, //11
            delivery_status: 1, //12
            interrupt_input_pin_polarity: 1, //13
            remote_irr: 1, //14
            trigger_mode: 1, //15
            mask: 1, //16
            reserved2: 15; //17-31
}LINT0_REG, * LINT0_REG_PTR;

typedef struct lint1_reg
{
    UINT32  vector: 8, //0-7
            delivery_mode: 3, //8-10
            reserved1: 1, //11
            delivery_status: 1, //12
            interrupt_input_pin_polarity: 1, //13
            remote_irr: 1, //14
            trigger_mode: 1, //15
            mask: 1, //16
            reserved2: 15; //17-31
}LINT1_REG, * LINT1_REG_PTR;

typedef struct error_reg
{
    UINT32  vector: 8, //0-7
            reserved1: 4, //8-11
            delivery_status: 1, //12
            reserved2: 3, //13-15
            mask: 1, //16
            reserved3: 15; //17-31
}ERROR_REG, * ERROR_REG_PTR;

typedef struct performance_monitor_count_reg
{
    UINT32  vector: 8, //0-7
            delivery_mode: 3, //8-10
            reserved1: 1, //11
            delivery_status: 1, //12
            reserved2: 3, //13-15
            mask: 1, //16
            reserved3: 15; //17-31
}PERFORMANCE_MONITOR_COUNT_REG, * PERFORMANCE_MONITOR_COUNT_REG_PTR;

typedef struct thermal_sensor_reg
{
    UINT32  vector: 8, //0-7
            delivery_mode: 3, //8-10
            reserved1: 1, //11
            delivery_status: 1, //12
            reserved2: 3, //13-15
            mask: 1, //16
            reserved3: 15; //17-31
}THERMAL_SENSOR_REG, * THERMAL_SENSOR_REG_PTR;

typedef struct error_status_reg
{
    UINT32  send_checksum_error: 1, //0
            receive_checksum_error: 1, //1
            send_accept_error: 1, //2
            receive_accept_error: 1, //3
            reserved1: 1, //4
            send_illegal_vector: 1, //5
            received_illegal_vector: 1, //6
            illegal_register_address: 1, //7
            reserved2: 24; //8-31
}ERROR_STATUS_REG, * ERROR_STATUS_REG_PTR;

typedef struct local_apic_version_reg
{
    UINT32  version: 8, //0-7/* 1XH = Local APIC, 0XH = 82489DX External APIC */
            reserved1: 8, //8-15
            max_lvt_entry: 8, //16-23
            reserved2: 8; //24-31
}LOCAL_APIC_VERSION_REG, * LOCAL_APIC_VERSION_REG_PTR;

typedef struct logical_destination_reg
{
    UINT32  reserved1: 24, //0-23
            logical_apic_id: 8; //24-31
}LOGICAL_DESTINATION_REG, * LOGICAL_DESTINATION_REG_PTR;

typedef struct destination_format_reg
{
    UINT32  reserved1: 28, //0-27 all 1's
            model: 4; //28-31
}DESTINATION_FORMAT_REG, * DESTINATION_FORMAT_REG_PTR;

typedef struct arbitration_priority_reg
{
    UINT32  arbitration_priority_subclass: 4, //0-3
            arbitration_priority: 4, //4-7
            reserved: 24; //8-31
}ARBITRATION_PRIORITY_REG, * ARBITRATION_PRIORITY_REG_PTR;

typedef struct task_priority_reg
{
    UINT32  task_priority_subclass: 4, //0-3
            task_priority: 4, //4-7
            reserved: 24; //8-31
}TASK_PRIORITY_REG, *TASK_PRIORITY_REG_PTR;

typedef struct processor_priority_reg
{
    UINT32  processor_priority_subclass: 4, //0-3
            processor_priority: 4, //4-7
            reserved: 24; //8-31
}PROCESSOR_PRIORITY_REG, *PROCESSOR_PRIORITY_REG_PTR;

typedef union interrupt_request_reg
{
    UINT32  vectors[8]; //16-255  32*8 = 256
    UINT16  reserved: 16; //0-15
}INTERRUPT_REQUEST_REG, * INTERRUPT_REQUEST_REG_PTR;

typedef union in_service_reg
{
    UINT16 reserved: 16; //0-15
    UINT32 vectors[8]; //16-255  32*8 = 256
}IN_SERVICE_REG, * IN_SERVICE_REG_PTR;

typedef union trigger_mode_reg
{
    UINT16 reserved: 16; //0-15
    UINT32 vectors[8]; //16-255  32*8 = 256
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


/* Declaration of the variables defined in apic.c */
extern IA32_APIC_BASE_MSR_PTR lapic_base_msr;

/* Functions */

int DetectAPIC(UINT8 cpu_id);
void UseAPIC(int enable);
void InitAPIC(void);
void SendEndOfInterrupt(int int_no);
void RelocateBaseLAPICAddress(UINT32 addr);
void InitSmp(void);
INT16 IssueInterprocessorInterrupt(UINT32 vector, UINT32 apic_id, enum ICR_DELIVERY_MODE delivery_mode,
                enum ICR_DESTINATION_SHORTHAND destination_shorthand, BYTE init_de_assert);

#endif
