/*!
 \file		kernel/interrupt.c
 \brief	Generic interrupt handler - architecture independent		
*/
#include <ace.h>
#include <ds/list.h>
#include <kernel/interrupt.h>
#include <kernel/debug.h>
#include <kernel/mm/kmem.h>

/*define this to print debug problems*/
//#define DEBUG_INTERRUPT

/* External functions present in i386/pic/apic.c */
extern UINT32 GetInterruptPriorityLevel(void);
extern UINT32 SetInterruptPriorityLevel(UINT32 ipl);

/*! \brief Raise the interrupt level to block any interrupts equal to or below this level.
 *  \param level - Level to which the current ipl should be raised.
 */
IRQ_PRIORITY_LEVELS RaiseInterruptPriorityLevel(IRQ_PRIORITY_LEVELS level)
{
	IRQ_PRIORITY_LEVELS ipl;

	ipl = GetInterruptPriorityLevel();
	if( level > ipl )
		panic("RaiseInterruptPriorityLevel: Decrementing priority level not allowed!");

	SetInterruptPriorityLevel(level);

	return ipl;
}

/*! \brief Restore the interrupt level after performing the required task.
 *  \param level - Level to which the current ipl should be restored.
 */
void RestoreInterruptPriorityLevel(IRQ_PRIORITY_LEVELS level)
{
	int ipl = GetInterruptPriorityLevel();
	if( level < ipl )
		panic("RestoreInterruptPriorityLevel: Incrementing priority level not allowed!");

	SetInterruptPriorityLevel(level);
}

extern void SendEndOfInterrupt(int);

INTERRUPT_HANDLER interrupt_handlers[MAX_INTERRUPTS];

static void InitInterruptHandler(INTERRUPT_HANDLER_PTR handler);

/*!
 * \brief Generic interrupt hander
 * \param reg - interrupt context (registers)

	All the archtecture specific interrupt stubs call this function.
	This function calls appropriate interrupt service routines defined in interrupt_routines[].
	It also returns EOI(End of Interrupt) to the PIC, so that PIC can now get interrupts.
 */
void InterruptHandler(REGS_PTR reg)
{
	INTERRUPT_INFO interrupt_info;
	INTERRUPT_HANDLER_PTR handler;
	LIST_PTR tmp;

#if ARCH == i386
	/*! In x86 the first 32 interrupts are exceptions and they are handled separately, so decrement by 32 to get correct interrupt number*/
	reg->int_no -= 32;
	interrupt_info.regs = reg;
#endif

	handler = &interrupt_handlers[reg->int_no];
	interrupt_info.interrupt_number = reg->int_no;
	/*TODO add code to include other interrupt details also*/
	
	if ( handler->isr == NULL )
	{
		#ifdef DEBUG_INTERRUPT
			kprintf("Interrupt recieved from %d but has no handler\n", reg->int_no);
		#endif
	}
	else
	{
		if ( handler->isr(&interrupt_info, handler->isr_argument) == ISR_CONTINUE_PROCESSING )
		{
			LIST_FOR_EACH(tmp, &interrupt_handlers[reg->int_no].next_isr )
			{
				handler = STRUCT_ADDRESS_FROM_MEMBER(tmp, INTERRUPT_HANDLER, next_isr );
				if ( handler->isr(&interrupt_info, handler->isr_argument) != ISR_CONTINUE_PROCESSING )
					break;
			}
		}
	}
	SendEndOfInterrupt( reg->int_no );
}

/*! Installs a custom IRQ handler for the given IRQ 
 * \param interrupt_number - interrupt number
 * \param isr_handler	- handler for the interrupt number
 * \param custom_argument - argument to be passed when the handler is called along with interrupt context
 */
void InstallInterruptHandler(int interrupt_number, ISR_HANDLER isr_handler, void * custom_argument)
{
	INTERRUPT_HANDLER_PTR handler;
	if ( interrupt_handlers[interrupt_number].isr == NULL )
	{
		handler = &interrupt_handlers[interrupt_number];
		InitList( &handler->next_isr );
	}
	else
	{
		handler = kmalloc( sizeof(INTERRUPT_HANDLER), 0 );
		InitInterruptHandler( handler );
		AddToListTail( &interrupt_handlers[interrupt_number].next_isr, &handler->next_isr);
	}
	handler->isr = isr_handler;
	handler->isr_argument = custom_argument;
}

/*!  This clears the handler for a given IRQ 
 * \param interrupt_number - interrupt number
 * \param isr_handler - function pointer for the interrupt number to uninstall
 */
void UninstallInterruptHandler(int interrupt_number, ISR_HANDLER isr_handler)
{
	INTERRUPT_HANDLER_PTR handler = NULL;
	if ( interrupt_handlers[interrupt_number].isr == isr_handler )
	{
		INTERRUPT_HANDLER_PTR next_handler, new_handler;
		next_handler = STRUCT_ADDRESS_FROM_MEMBER(&interrupt_handlers[interrupt_number].next_isr, INTERRUPT_HANDLER, next_isr );
		new_handler = STRUCT_ADDRESS_FROM_MEMBER(&next_handler->next_isr, INTERRUPT_HANDLER, next_isr );
		/*if only one interrupt handler*/
		if ( &interrupt_handlers[interrupt_number] == next_handler )
		{
			interrupt_handlers[interrupt_number].isr = NULL;
			interrupt_handlers[interrupt_number].isr_argument = NULL;
			return;
		}
		interrupt_handlers[interrupt_number].isr = new_handler->isr;
		interrupt_handlers[interrupt_number].isr_argument = new_handler->isr;
		RemoveFromList( &new_handler->next_isr);
		kfree( new_handler );
	}
	else
	{
		LIST_PTR tmp;
		LIST_FOR_EACH(tmp, &interrupt_handlers[interrupt_number].next_isr )
		{
			handler = STRUCT_ADDRESS_FROM_MEMBER(tmp, INTERRUPT_HANDLER, next_isr );
			if ( handler->isr == isr_handler )
				break;
		}
		assert ( handler != NULL );
		RemoveFromList( &handler->next_isr);
		kfree( handler );
	}
}

/*! Initializes a interrupt handler structure with default values (NULL)
 * \param handler - interrupt handler to initialize.
 */
static void InitInterruptHandler(INTERRUPT_HANDLER_PTR handler)
{
	handler->isr = NULL;
	handler->isr_argument = NULL;
	InitList( &handler->next_isr );
}

