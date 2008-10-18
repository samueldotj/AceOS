/*! \file 	kernel/arch.h
    \brief 	Architecture specific function declartions
*/

#ifndef ARCH__H
#define ARCH__H

#include <ace.h>
#include <string.h>
#include <kernel/error.h>
#include <kernel/io.h>
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/multiboot.h>
#include <kernel/parameter.h>
#include <kernel/processor.h>
#include <kernel/ioapic.h>
#include <kernel/interrupt.h>
#include <kernel/apic.h>
#include <kernel/pit.h>
#include <kernel/acpi/acpi.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/virtual_page.h>


#ifdef __cplusplus
    extern "C" {
#endif

void panic(char * message);
void InitArchPhase1(MULTIBOOT_INFO_PTR mbi);
void InitArchPhase2(MULTIBOOT_INFO_PTR mbi);
void InitSecondaryProcessors();

void InvalidateTlb(void * va);
void InvalidateAllTlb();
void FlushCpuCache(BOOLEAN write_back);

void ArchHalt();

void MaskInterrupt(BYTE interrupt_number);

void StartTimer(UINT32 frequency, BYTE periodic);

extern UINT16 master_processor_id;
extern UINT32 cpu_frequency;

extern VADDR multiboot_module_va_start;
extern VADDR multiboot_module_va_end;

#ifdef __cplusplus
	}
#endif

#endif
