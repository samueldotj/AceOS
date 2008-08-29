/*! \file 	kernel/arch.h
    \brief 	Architecture specific function declartions
*/

#ifndef ARCH__H
#define ARCH__H

#include <ace.h>
#include <kernel/multiboot.h>

#ifdef __cplusplus
    extern "C" {
#endif

void panic(char * message);
void InitArchPhase1(MULTIBOOT_INFO_PTR mbi);
void InitArchPhase2(MULTIBOOT_INFO_PTR mbi);
void InitPrintf();
void InitSmp();

void InvalidateTlb(void * va);
void InvalidateAllTlb();
void FlushCpuCache(BOOLEAN write_back);

void ArchHalt();

#ifdef __cplusplus
	}
#endif


#endif
