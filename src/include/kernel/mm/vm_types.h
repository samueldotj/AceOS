/*!
  \file		kernel/mm/vm_types.h
  \author	Samuel (samueldotj@gmail.com)
  \version 	3.0
  \date	
  			Created: 13-Jun-2008 4:12PM
  			Last modified: 13-Jun-2008 4:12PM
  \brief	all the typedefs of vm and pm.
*/
#ifndef _VM_TYPES_H
#define _VM_TYPES_H

typedef struct vm_data 			VM_DATA, 		* VM_DATA_PTR;

typedef struct vm_protection 	VM_PROTECTION,  * VM_PROTECTION_PTR;

typedef struct virtual_map 		VIRTUAL_MAP, 	* VIRTUAL_MAP_PTR;
typedef struct vm_descriptor 	VM_DESCRIPTOR,  * VM_DESCRIPTOR_PTR;
typedef struct vm_unit 			VM_UNIT, 		* VM_UNIT_PTR;
typedef struct vm_vtop 			VM_VTOP, 		* VM_VTOP_PTR;

typedef struct physical_map 	PHYSICAL_MAP, 	* PHYSICAL_MAP_PTR;

typedef struct virtual_page 	VIRTUAL_PAGE, 	* VIRTUAL_PAGE_PTR;
typedef struct va_map			VA_MAP, 		* VA_MAP_PTR;

#endif
