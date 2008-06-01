/*!
  \file	page_init.c
  \author	Samuel (samueldotj@gmail.com)
  \version 	3.0
  \date	
  			Created: 21-Mar-2008 5:13PM
  			Last modified: Tue Apr 01, 2008  11:10PM
  \brief	Ace Kernel memory management - i386 page directory/table management
	This file initializes the kernel page table it is in separate file because it executes in protected not in paged mode.
*/
#include <ace.h>
#include <kernel/i386/pagetab.h>
#include <kernel/mm/vm.h>
#include <kernel/debug.h>


