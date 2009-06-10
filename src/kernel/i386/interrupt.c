/*!
	\file	kernel/i386/interrupt.c
	\brief	i386 specific interrupt controller routines.
*/
#include <ace.h>
#include <kernel/arch.h>
#include <kernel/parameter.h>
#include <kernel/pm/thread.h>
#include <kernel/i386/i386.h>

static int use_pic_8259 = 0;

/*! Initialize interrupt controllers
*/
void InitInterruptControllers()
{
	ACPI_TABLE_MADT *madt_ptr;
	ACPI_SUBTABLE_HEADER *sub_header, *table_end;
	ACPI_STATUS result;
	int i;
	
	/* disable all interrupts*/
	asm volatile("cli");

	memset(interrupt_handlers, 0, sizeof(interrupt_handlers));
	
	/*! Initialize the PIC*/
	InitPic(PIC_STARTING_VECTOR_NUMBER);
	/*initialize the legacy IRQ to interrupt mapping table*/
	for(i=0;i<16;i++)
		legacy_irq_redirection_table[i]=i;

	count_ioapic = 0;

	/*! try to read APIC table*/
	result = AcpiGetTable ("APIC", 1, (ACPI_TABLE_HEADER**)(&madt_ptr));
	if ( result == AE_OK )
	{
		kprintf("LAPIC Address %p [%s]\n", madt_ptr->Address, madt_ptr->Flags&1 ? "APIC and Dual 8259 Support" : "Only APIC" );
		/* if the machine has both 8259 and IOAPIC support disable the 8259*/
		if ( madt_ptr->Flags & 1 )
			use_pic_8259 = 0;
	
		sub_header = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)madt_ptr) + sizeof(ACPI_TABLE_MADT) );
		table_end = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)madt_ptr) + madt_ptr->Header.Length );

		while ( sub_header < table_end )
		{
			if ( sub_header->Type == ACPI_MADT_TYPE_IO_APIC )
			{
				ACPI_MADT_IO_APIC *p = ( ACPI_MADT_IO_APIC * ) sub_header;
				kprintf("IOAPIC ID %d IOAPIC Physical Address = %p GlobalIRQBase %d\n", p->Id, p->Address, p->GlobalIrqBase);
				
				/*!initialize IOAPIC structures*/
				ioapic[count_ioapic].ioapic_id = p->Id;
				ioapic[count_ioapic].end_irq = ioapic[count_ioapic].start_irq = p->GlobalIrqBase;
				ioapic[count_ioapic].base_physical_address = (void *) p->Address;
				ioapic[count_ioapic].base_virtual_address = (void *) MapPhysicalMemory(&kernel_map, p->Address, PAGE_SIZE, 0, PROT_READ | PROT_WRITE);
				
				/*! initialize the ioapic chip and get total irq supported*/
				ioapic[count_ioapic].end_irq += InitIoApic(ioapic[count_ioapic].base_virtual_address, ioapic[count_ioapic].start_irq+IOAPIC_STARTING_VECTOR_NUMBER);
							
				count_ioapic++;
			}
			else if ( sub_header->Type == ACPI_MADT_TYPE_INTERRUPT_OVERRIDE )
			{
				ACPI_MADT_INTERRUPT_OVERRIDE * override = (ACPI_MADT_INTERRUPT_OVERRIDE *)sub_header;
				legacy_irq_redirection_table[override->SourceIrq] = override->GlobalIrq;
			}
			else
			{
				/* if found someother structure just print the type for debugging*/
				if ( sub_header->Type != ACPI_MADT_TYPE_LOCAL_APIC )
					kprintf("ACPI MADT Structure type %d is not yet implemented\n", sub_header->Type);
			}

			sub_header = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)sub_header) + sub_header->Length );
		}
	}
	else
	{
		#ifdef CONFIG_SMP
			kprintf("AcpiGetTable() failed Code=%d, assuming uniprocessor and using 8259 PIC\n", result);
		#endif
		use_pic_8259 = 1;
	}
	
	/*! disable PIC if we dont need it*/
	if ( !use_pic_8259 )
		MaskPic();
	
	/*! enable interrupts*/
	asm volatile("sti");
}

/*! Sends EOI to either 8259 or LAPIC
	\param int_no - interrupt number to acknowledge
*/
void SendEndOfInterrupt(int int_no)
{
	if ( use_pic_8259 )
		SendEndOfInterruptTo8259(int_no);
	else
		SendEndOfInterruptToLapic(int_no);
}
