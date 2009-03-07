/*!
  \file		kernel/i386/processor.h
  \brief	Processor data structure - architecture dependent part

		Architecture depended and independed processor structures are maintained using parallel arrays.
		Architecture depended processor structure is declared inside arch/processor.h
*/

#ifndef _PROCESSOR_I386_H_
#define _PROCESSOR_I386_H_

#include <ace.h>
#include <kernel/processor.h>
#include <kernel/i386/apic.h>
#include <kernel/i386/cpuid.h>
#include <kernel/i386/tss.h>

/*! Data structure for architecture independed part of a i386 processor(which supports CPUID and has LAPIC)*/
typedef struct processor_i386
{
	CPUID_INFO			cpuid;					/*! CPUID data returned by the processor*/
	UINT16				apic_id;				/*! APIC id of the CPU, we cant use CPUID data until the CPU starts*/
	TSS					tss;					/*! task state segment for this cpu*/	
}PROCESSOR_I386, *PROCESSOR_I386_PTR;

/*Processors are indexed by using APIC ID*/
extern PROCESSOR_I386 processor_i386[MAX_PROCESSORS];

#endif
