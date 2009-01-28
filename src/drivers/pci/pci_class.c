/*!
	\file	drivers/pci/pci_vendor_database.c
	\brief	Contains compiled class code list
*/

#include <ace.h>
#include <stdlib.h>
#include <kernel/debug.h>
#include "pci.h"

typedef struct pci_progam_interface PCI_PROGAM_INTERFACE, * PCI_PROGAM_INTERFACE_PTR;
typedef struct pci_subclass PCI_SUBCLASS, * PCI_SUBCLASS_PTR;
typedef struct pci_class PCI_CLASS, * PCI_CLASS_PTR;

typedef struct pci_vendor PCI_VENDOR, * PCI_VENDOR_PTR;

struct pci_progam_interface
{
	BYTE						code;
	char *						description;
};
struct pci_subclass
{
	BYTE						code;
	char *						description;
	
	PCI_PROGAM_INTERFACE_PTR	interfaces;
	BYTE						interface_count;
};
struct pci_class
{
	BYTE						code;
	char *						description;
	
	PCI_SUBCLASS_PTR			subclasses;
	BYTE						subclass_count;
};

#define SIZEOF_SUBCLASS(subclass_array)		( sizeof(subclass_array)/sizeof(PCI_SUBCLASS) )
#define SIZEOF_PROGRAM_IF(prog_if_array)	( sizeof(prog_if_array)/sizeof(PCI_PROGAM_INTERFACE) )

/*static name description for each class code*/

PCI_SUBCLASS pci_class0_subclass[]	=
{
	{0x0,	"Non-VGA", 			NULL},
	{0x1,	"VGA Compatible", 	NULL}
};
PCI_SUBCLASS pci_class1_subclass[]	=
{
	{0x0,	"SCSI",		NULL},
	{0x1,	"IDE",		NULL},
	{0x2,	"Floppy",	NULL},
	{0x3,	"IPI",		NULL},
	{0x4,	"Raid",		NULL},
	{0x80,	"Other",	NULL}
};
PCI_SUBCLASS pci_class2_subclass[]	=
{
	{0x0,	"Ethernet", 	NULL},
	{0x1,	"Token ring",	NULL},
	{0x2,	"FDDI",			NULL},
	{0x3,	"ATM",			NULL},
	{0x80,	"Other",		NULL}
};
PCI_PROGAM_INTERFACE pci_class3_subclass0_interface[]=
{
	{0x0,	"VGA"},
	{0x1,	"8514"}
};
PCI_SUBCLASS pci_class3_subclass[]	=
{
	{0x0,	"PC Compatible", 	pci_class3_subclass0_interface},
	{0x2,	"XGA",				NULL},
	{0x80,	"Other",			NULL}
};
PCI_SUBCLASS pci_class4_subclass[]	=
{
	{0x0,	"Video", 	NULL},
	{0x1,	"Audio",	NULL},
	{0x80,	"Other",	NULL}
};
PCI_SUBCLASS pci_class5_subclass[]	=
{
	{0x0,	"RAM", 		NULL},
	{0x1,	"Flash",	NULL},
	{0x80,	"Other",	NULL}
};
PCI_SUBCLASS pci_class6_subclass[]	=
{
	{0x0,	"Host/PCI", 		NULL},
	{0x1,	"PCI/ISA",			NULL},
	{0x2,	"PCI/EISA",			NULL},
	{0x3,	"PCI/Micro Channel",NULL},
	{0x4,	"PCI/PCI",			NULL},
	{0x5,	"PCI/PCMCIA",		NULL},
	{0x6,	"PCI/NuBus",		NULL},
	{0x7,	"PCI/CardBus",		NULL},
	{0x80,	"Other",			NULL}
};
PCI_PROGAM_INTERFACE pci_class7_subclass0_interface[]=
{
	{0x0,	"Generic XT Compatible"},
	{0x1,	"16450 Compatible"},
	{0x2,	"16550 Compatible"}
};
PCI_PROGAM_INTERFACE pci_class7_subclass1_interface[]=
{
	{0x0,	"Standard"},
	{0x1,	"Bidirectional"},
	{0x2,	"ECP 1.X Compliant"}
};

PCI_SUBCLASS pci_class7_subclass[]	=
{
	{0x0,	"Serial", 		pci_class7_subclass0_interface, SIZEOF_PROGRAM_IF(pci_class7_subclass0_interface)},	
	{0x1,	"Parallel",		pci_class7_subclass1_interface, SIZEOF_PROGRAM_IF(pci_class7_subclass1_interface)},
	{0x80,	"Other",		NULL}
};

PCI_PROGAM_INTERFACE pci_class8_subclass0_interface[]=
{
	{0x0,	"Generic 8259"},
	{0x1,	"ISA"},
	{0x2,	"PCI"}
};
PCI_PROGAM_INTERFACE pci_class8_subclass1_interface[]=
{
	{0x0,	"Generic 8259"},
	{0x1,	"ISA"},
	{0x2,	"EISA"}
};
PCI_PROGAM_INTERFACE pci_class8_subclass2_interface[]=
{
	{0x0,	"Generic 8259"},
	{0x1,	"ISA"},
	{0x2,	"EISA"}
};
PCI_PROGAM_INTERFACE pci_class8_subclass3_interface[]=
{
	{0x0,	"Generic"},
	{0x1,	"ISA"}
};

PCI_SUBCLASS pci_class8_subclass[]	=
{
	{0x0,	"PIC", 		pci_class8_subclass0_interface, SIZEOF_PROGRAM_IF(pci_class8_subclass0_interface)},
	{0x1,	"DMA",		pci_class8_subclass1_interface, SIZEOF_PROGRAM_IF(pci_class8_subclass1_interface)},
	{0x2,	"Timer",	pci_class8_subclass2_interface, SIZEOF_PROGRAM_IF(pci_class8_subclass2_interface)},
	{0x3,	"RTC",		pci_class8_subclass3_interface, SIZEOF_PROGRAM_IF(pci_class8_subclass3_interface)},
	{0x80,	"Other",	NULL}
};

PCI_SUBCLASS pci_class9_subclass[]	=
{
	{0x0,	"Keyboard", 		NULL},
	{0x1,	"Digitizer (Pen)",	NULL},
	{0x2,	"Mouse", 			NULL},
	{0x80,	"Other",			NULL}
};

PCI_SUBCLASS pci_class10_subclass[]	=
{
	{0x0,	"Generic", 	NULL},
	{0x80,	"Other",	NULL}
};

PCI_SUBCLASS pci_class11_subclass[]	=
{
	{0x0,	"i386", 		NULL},
	{0x1,	"i486", 		NULL},
	{0x2,	"Pentium", 		NULL},
	{0x10,	"Alpha", 		NULL},
	{0x20,	"Power PC", 	NULL},
	{0x80,	"Co-processor",	NULL}
};

PCI_SUBCLASS pci_class12_subclass[]	=
{
	{0x0,	"Firewire (IEEE 1394)", 				NULL},
	{0x1,	"ACCESS.bus", 							NULL},
	{0x2,	"SSA (Serial Storage Archetecture)", 	NULL},
	{0x3,	"USB (Universal Serial Bus)", 			NULL},
	{0x4,	"Fibre Channel", 						NULL}
};

PCI_CLASS pci_class_table[] =
{
	{0x0, "Pre-2.0 PCI Specification Device", 	pci_class0_subclass, 	SIZEOF_SUBCLASS(pci_class0_subclass)},
	{0x1, "Mass Storage Controller", 			pci_class1_subclass, 	SIZEOF_SUBCLASS(pci_class1_subclass)},
	{0x2, "Network Controller", 				pci_class2_subclass, 	SIZEOF_SUBCLASS(pci_class2_subclass)},
	{0x3, "Display Controller", 				pci_class3_subclass, 	SIZEOF_SUBCLASS(pci_class3_subclass)},
	{0x4, "Multimedia Device", 					pci_class4_subclass, 	SIZEOF_SUBCLASS(pci_class4_subclass)},
	{0x5, "Memory Controller", 					pci_class5_subclass, 	SIZEOF_SUBCLASS(pci_class5_subclass)},
	{0x6, "Bridge Device", 						pci_class6_subclass, 	SIZEOF_SUBCLASS(pci_class6_subclass)},
	{0x7, "Simple Communications Controller", 	pci_class7_subclass, 	SIZEOF_SUBCLASS(pci_class7_subclass)},
	{0x8, "Base Systems Peripheral", 			pci_class8_subclass, 	SIZEOF_SUBCLASS(pci_class8_subclass)},
	{0x9, "Input Device", 						pci_class9_subclass, 	SIZEOF_SUBCLASS(pci_class8_subclass)},
	{0xa, "Docking Station",					pci_class10_subclass, 	SIZEOF_SUBCLASS(pci_class8_subclass)},
	{0xb, "Processor", 							pci_class11_subclass, 	SIZEOF_SUBCLASS(pci_class8_subclass)},
	{0xc, "Serial Bus Controller",				pci_class12_subclass, 	SIZEOF_SUBCLASS(pci_class8_subclass)},
};

struct pci_vendor
{
	UINT16	id;
	char *	short_name;
	char *	full_name;
};


typedef struct pci_vendor_device PCI_VENDOR_DEVICE, *PCI_VENDOR_DEVICE_PTR;
struct pci_vendor_device 
{
	UINT16	vendor_id;
	UINT16	device_id;
	char *	chip;
	char *	chip_description;
};

/*! Returns descriptive name for the given class/subclass/program_interface
	\return 0 on success
*/
int get_pci_class_string(BYTE class_code, BYTE subclass_code, BYTE prog_if_code, char ** class_name, char ** subclass_name, char ** prog_if_name )
{
	int i;
	
	if ( class_name )
		* class_name = "";
	if ( subclass_name )
		* subclass_name = "";
	if ( prog_if_name )
		* prog_if_name = "";

	/*return failure if invalid class code*/
	if( class_code >= sizeof(pci_class_table)/sizeof(PCI_CLASS) )
		return 1;
	if ( class_name )
		* class_name = pci_class_table[class_code].description;
	for(i=0; i<pci_class_table[class_code].subclass_count; i++ )
	{
		int j;
		PCI_SUBCLASS_PTR sub_class = &pci_class_table[class_code].subclasses[i];
		assert(sub_class != NULL);
		if ( subclass_code == sub_class->code )
		{
			if ( subclass_name )
				* subclass_name = sub_class->description;
			for(j=0; j< sub_class->interface_count; j++ )
			{
				PCI_PROGAM_INTERFACE_PTR prog_if = &sub_class->interfaces[j];
				if( prog_if_code == prog_if->code )
				{
					if ( prog_if_name )
						* prog_if_name = prog_if->description;
				}
			}
		}
	}
	return 0;
}
