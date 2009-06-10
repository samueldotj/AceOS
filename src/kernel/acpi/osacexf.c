/*! ACPI-CA OS Services Layer - External Interface Definition
*/

#include <stdlib.h>
#include <stdarg.h>
#include <ace.h>
#include <sync/spinlock.h>
#include <kernel/io.h>
#include <kernel/interrupt.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/kmem.h>

#include "acpi.h"
#include "amlcode.h"
#include "acparser.h"
#include "acdebug.h"

#define _COMPONENT          ACPI_OS_SERVICES
        ACPI_MODULE_NAME    ("osacexf")

typedef struct acpi_isr_handler
{
	ACPI_OSD_HANDLER	handler;
	void *				context;
}ACPI_ISR_HANDLER, * ACPI_ISR_HANDLER_PTR;

#define ACPI_MAX_INIT_TABLES (16)
static ACPI_TABLE_DESC      Tables[ACPI_MAX_INIT_TABLES];
int InitACPI()
{
	ACPI_STATUS result;
	result = AcpiInitializeSubsystem();
	if ( result != AE_OK )
	{
		kprintf("AcpiInitializeSubsystem() failed %x\n", result);
		return -1;
	}
	result = AcpiInitializeTables (Tables, ACPI_MAX_INIT_TABLES, TRUE);
    if ( result != AE_OK )
	{
		kprintf("AcpiInitializeTables() failed %x\n", result);
		return -1;
	}
    result = AcpiReallocateRootTable ();
	if ( result != AE_OK )
	{
		kprintf("AcpiReallocateRootTable() failed %x\n", result);
		return -1;
	}
	result = AcpiLoadTables();
	if ( result != AE_OK )
	{
		kprintf("AcpiLoadTables() failed %x\n", result);
		return -1;
	}
	result = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
	if ( result != AE_OK )
	{
		kprintf("AcpiEnableSubsystem() failed %x\n", result);
		return -1;
	}
	result = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
	if ( result != AE_OK )
	{
		kprintf("AcpiInitializeObjects() failed %x\n", result);
		return -1;
	}
	
	return 0;
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsInitialize, AcpiOsTerminate
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Init and terminate.  Nothing to do.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsInitialize (void)
{
	return AE_OK;
}


ACPI_STATUS
AcpiOsTerminate (void)
{
    return AE_OK;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetRootPointer
 *
 * PARAMETERS:  None
 *
 * RETURN:      RSDP physical address
 *
 * DESCRIPTION: Gets the root pointer (RSDP)
 *
 *****************************************************************************/

ACPI_PHYSICAL_ADDRESS
AcpiOsGetRootPointer (
    void)
{
	ACPI_NATIVE_UINT tbl_address;
	if ( AcpiFindRootPointer( &tbl_address ) == AE_OK )
		return tbl_address;
	else
		return  0;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsPredefinedOverride
 *
 * PARAMETERS:  InitVal     - Initial value of the predefined object
 *              NewVal      - The new value for the object
 *
 * RETURN:      Status, pointer to value.  Null pointer returned if not
 *              overriding.
 *
 * DESCRIPTION: Allow the OS to override predefined names
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsPredefinedOverride (
    const ACPI_PREDEFINED_NAMES *InitVal,
    ACPI_STRING                 *NewVal)
{

    if (!InitVal || !NewVal)
    {
        return (AE_BAD_PARAMETER);
    }

    *NewVal = NULL;
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsTableOverride
 *
 * PARAMETERS:  ExistingTable   - Header of current table (probably firmware)
 *              NewTable        - Where an entire new table is returned.
 *
 * RETURN:      Status, pointer to new table.  Null pointer returned if no
 *              table is available to override
 *
 * DESCRIPTION: Return a different version of a table if one is available
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsTableOverride (
    ACPI_TABLE_HEADER       *ExistingTable,
    ACPI_TABLE_HEADER       **NewTable)
{

    if (!ExistingTable || !NewTable)
    {
        return (AE_BAD_PARAMETER);
    }

    *NewTable = NULL;

#ifdef ACPI_EXEC_APP

    /* This code exercises the table override mechanism in the core */

    if (ACPI_COMPARE_NAME (ExistingTable->Signature, ACPI_SIG_DSDT))
    {
        /* override DSDT with itself */

        *NewTable = AcpiGbl_DbTablePtr;
    }
    return (AE_OK);
#else
    return AE_NO_ACPI_TABLES;
#endif
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadable
 *
 * PARAMETERS:  Pointer             - Area to be verified
 *              Length              - Size of area
 *
 * RETURN:      TRUE if readable for entire length
 *
 * DESCRIPTION: Verify that a pointer is valid for reading
 *
 *****************************************************************************/

BOOLEAN
AcpiOsReadable (
    void                    *Pointer,
    ACPI_SIZE               Length)
{
	VA_STATUS result = GetVirtualRangeStatus((VADDR)Pointer, Length);
	return ( result == VA_WRITEABLE || result == VA_READABLE );
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsWritable
 *
 * PARAMETERS:  Pointer             - Area to be verified
 *              Length              - Size of area
 *
 * RETURN:      TRUE if writable for entire length
 *
 * DESCRIPTION: Verify that a pointer is valid for writing
 *
 *****************************************************************************/

BOOLEAN
AcpiOsWritable (
    void                    *Pointer,
    ACPI_SIZE               Length)
{
    VA_STATUS result = GetVirtualRangeStatus((VADDR)Pointer, Length);
	return ( result == VA_WRITEABLE );
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsRedirectOutput
 *
 * PARAMETERS:  Destination         - An open file handle/pointer
 *
 * RETURN:      None
 *
 * DESCRIPTION: Causes redirect of AcpiOsPrintf and AcpiOsVprintf
 *
 *****************************************************************************/

void
AcpiOsRedirectOutput (
    void                    *Destination)
{

    
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsPrintf
 *
 * PARAMETERS:  fmt, ...            Standard printf format
 *
 * RETURN:      None
 *
 * DESCRIPTION: Formatted output
 *
 *****************************************************************************/

void ACPI_INTERNAL_VAR_XFACE
AcpiOsPrintf (
    const char              *Fmt,
    ...)
{
    va_list                 Args;


    va_start (Args, Fmt);

    AcpiOsVprintf (Fmt, Args);

    va_end (Args);
    return;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsVprintf
 *
 * PARAMETERS:  fmt                 Standard printf format
 *              args                Argument list
 *
 * RETURN:      None
 *
 * DESCRIPTION: Formatted output with argument list pointer
 *
 *****************************************************************************/

void
AcpiOsVprintf (
    const char              *Fmt,
    va_list                 Args)
{
    INT32                   Count = 0;
    UINT8                   Flags;


    Flags = AcpiGbl_DbOutputFlags;
    if (Flags & ACPI_DB_REDIRECTABLE_OUTPUT)
    {
        /* Output is directable to either a file (if open) or the console */

		if ( 0 )
        //if (AcpiGbl_DebugFile)
        {
            /* Output file is open, send the output there */

            Count = vfprintf (0, Fmt, Args);
        }
        else
        {
            /* No redirection, send output to console (once only!) */

            Flags |= ACPI_DB_CONSOLE_OUTPUT;
        }
    }

    if (Flags & ACPI_DB_CONSOLE_OUTPUT)
    {
        Count = vfprintf (0, Fmt, Args);
    }

    return;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetLine
 *
 * PARAMETERS:  fmt                 Standard printf format
 *              args                Argument list
 *
 * RETURN:      Actual bytes read
 *
 * DESCRIPTION: Formatted input with argument list pointer
 *
 *****************************************************************************/

UINT32
AcpiOsGetLine (
    char                    *Buffer)
{
    return 0;
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsMapMemory
 *
 * PARAMETERS:  where               Physical address of memory to be mapped
 *              length              How much memory to map
 *
 * RETURN:      Pointer to mapped memory.  Null on error.
 *
 * DESCRIPTION: Map physical memory into caller's address space
 *
 *****************************************************************************/

void *
AcpiOsMapMemory (
    ACPI_PHYSICAL_ADDRESS   where,
    ACPI_SIZE               length)
{
	VADDR va = MapPhysicalMemory( &kernel_map, where, length, 0, PROT_READ | PROT_WRITE );
	if ( va )
	    return (ACPI_TO_POINTER ((ACPI_NATIVE_UINT) (va + where-PAGE_ALIGN(where)) ));
	else
		return NULL;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsUnmapMemory
 *
 * PARAMETERS:  where               Logical address of memory to be unmapped
 *              length              How much memory to unmap
 *
 * RETURN:      None.
 *
 * DESCRIPTION: Delete a previously created mapping.  Where and Length must
 *              correspond to a previous mapping exactly.
 *
 *****************************************************************************/

void
AcpiOsUnmapMemory (
    void                    *where,
    ACPI_SIZE               length)
{
	FreeVirtualMemory(&kernel_map, (VADDR)where, length, 0);
    return;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsAllocate
 *
 * PARAMETERS:  Size                Amount to allocate, in bytes
 *
 * RETURN:      Pointer to the new allocation.  Null on error.
 *
 * DESCRIPTION: Allocate memory.  Algorithm is dependent on the OS.
 *
 *****************************************************************************/

void *
AcpiOsAllocate (
    ACPI_SIZE               size)
{
	void * ret = kmalloc(size, 0);
    return ret;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsFree
 *
 * PARAMETERS:  mem                 Pointer to previously allocated memory
 *
 * RETURN:      None.
 *
 * DESCRIPTION: Free memory allocated via AcpiOsAllocate
 *
 *****************************************************************************/

void
AcpiOsFree (
    void                    *mem)
{
	kfree(mem);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsCreateSemaphore
 *
 * PARAMETERS:  InitialUnits        - Units to be assigned to the new semaphore
 *              OutHandle           - Where a handle will be returned
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Create an OS semaphore
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsCreateSemaphore (
    UINT32              MaxUnits,
    UINT32              InitialUnits,
    ACPI_HANDLE         *OutHandle)
{


    *OutHandle = (ACPI_HANDLE) 1;
    return AE_OK;
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsDeleteSemaphore
 *
 * PARAMETERS:  Handle              - Handle returned by AcpiOsCreateSemaphore
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Delete an OS semaphore
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsDeleteSemaphore (
    ACPI_HANDLE         Handle)
{

    if (!Handle)
    {
        return AE_BAD_PARAMETER;
    }

    return AE_OK;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsWaitSemaphore
 *
 * PARAMETERS:  Handle              - Handle returned by AcpiOsCreateSemaphore
 *              Units               - How many units to wait for
 *              Timeout             - How long to wait
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Wait for units
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWaitSemaphore (
    ACPI_HANDLE         Handle,
    UINT32              Units,
    UINT16              Timeout)
{
    return AE_OK;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsSignalSemaphore
 *
 * PARAMETERS:  Handle              - Handle returned by AcpiOsCreateSemaphore
 *              Units               - Number of units to send
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Send units
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsSignalSemaphore (
    ACPI_HANDLE         Handle,
    UINT32              Units)
{
    return AE_OK;
}


ACPI_STATUS
AcpiOsCreateLock (
    ACPI_SPINLOCK           *OutHandle)
{
	SPIN_LOCK_PTR spinlock = kmalloc( sizeof(SPIN_LOCK), 0);
	InitSpinLock(spinlock);
	
	*((SPIN_LOCK_PTR *)OutHandle) = spinlock;
    return AE_OK;
}

void
AcpiOsDeleteLock (
    ACPI_SPINLOCK           Handle)
{
    kfree(Handle);
}


ACPI_CPU_FLAGS
AcpiOsAcquireLock (
    ACPI_SPINLOCK             Handle)
{
    SpinLock(Handle);
    return (0);
}


void
AcpiOsReleaseLock (
    ACPI_SPINLOCK           Handle,
    ACPI_CPU_FLAGS          Flags)
{
    SpinUnlock(Handle);
}

ISR_RETURN_CODE AcpiInterruptHandler(INTERRUPT_INFO_PTR interrupt_info, void * arg)
{
	ACPI_ISR_HANDLER_PTR p = (ACPI_ISR_HANDLER_PTR) arg;
	assert ( p != NULL );
	p->handler(p->context);
	return ISR_CONTINUE_PROCESSING;
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsInstallInterruptHandler
 *
 * PARAMETERS:  InterruptNumber     Level handler should respond to.
 *              Isr                 Address of the ACPI interrupt handler
 *              ExceptPtr           Where status is returned
 *
 * RETURN:      Handle to the newly installed handler.
 *
 * DESCRIPTION: Install an interrupt handler.  Used to install the ACPI
 *              OS-independent handler.
 *
 *****************************************************************************/

UINT32
AcpiOsInstallInterruptHandler (
    UINT32                  InterruptNumber,
    ACPI_OSD_HANDLER        ServiceRoutine,
    void                    *Context)
{
	ACPI_ISR_HANDLER_PTR p = kmalloc( sizeof(ACPI_ISR_HANDLER), 0);
	if ( p == NULL )
		return AE_ERROR;
	
	InstallInterruptHandler( InterruptNumber, AcpiInterruptHandler, p);
	
    return AE_OK;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsRemoveInterruptHandler
 *
 * PARAMETERS:  Handle              Returned when handler was installed
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Uninstalls an interrupt handler.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsRemoveInterruptHandler (
    UINT32                  InterruptNumber,
    ACPI_OSD_HANDLER        ServiceRoutine)
{
	UninstallInterruptHandler( InterruptNumber, AcpiInterruptHandler);
    return AE_ERROR;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsExecute
 *
 * PARAMETERS:  Type            - Type of execution
 *              Function        - Address of the function to execute
 *              Context         - Passed as a parameter to the function
 *
 * RETURN:      Status.
 *
 * DESCRIPTION: Execute a new thread
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsExecute (
    ACPI_EXECUTE_TYPE       Type,
    ACPI_OSD_EXEC_CALLBACK  Function,
    void                    *Context)
{

//    _beginthread (Function, (unsigned) 0, Context);
    return 0;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsBreakpoint
 *
 * PARAMETERS:  Msg                 Message to print
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Print a message and break to the debugger.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsBreakpoint (
    char                    *Msg)
{

    if (Msg)
    {
        AcpiOsPrintf ("AcpiOsBreakpoint: %s ****\n", Msg);
    }
    else
    {
        AcpiOsPrintf ("At AcpiOsBreakpoint ****\n");
    }

    return AE_OK;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsStall
 *
 * PARAMETERS:  microseconds        To sleep
 *
 * RETURN:      Blocks until sleep is completed.
 *
 * DESCRIPTION: Sleep at microsecond granularity
 *
 *****************************************************************************/

void
AcpiOsStall (
    UINT32                  microseconds)
{

    if (microseconds)
    {
        //usleep (microseconds);
    }
    return;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsSleep
 *
 * PARAMETERS:  milliseconds        To sleep
 *
 * RETURN:      Blocks until sleep is completed.
 *
 * DESCRIPTION: Sleep at millisecond granularity
 *
 *****************************************************************************/

void
AcpiOsSleep (
    ACPI_INTEGER            milliseconds)
{

    //sleep (milliseconds / 1000);    /* Sleep for whole seconds */

    /*
     * Arg to usleep() must be less than 1,000,000 (1 second)
     */
    //usleep ((milliseconds % 1000) * 1000);      /* Sleep for remaining usecs */

    return;
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetTimer
 *
 * PARAMETERS:  None
 *
 * RETURN:      Current time in 100 nanosecond units
 *
 * DESCRIPTION: Get the current system time
 *
 *****************************************************************************/

UINT64
AcpiOsGetTimer (void)
{
	SYSTEM_TIME time;
	
	GetLocalTime( &time );
	/* Seconds * 10^7 = 100ns(10^-7), Microseconds(10^-6) * 10^1 = 100ns */
	return (((UINT64) time.second * 10000000) + ((UINT64) time.milliseconds * 10));
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsValidateInterface
 *
 * PARAMETERS:  Interface           - Requested interface to be validated
 *
 * RETURN:      AE_OK if interface is supported, AE_SUPPORT otherwise
 *
 * DESCRIPTION: Match an interface string to the interfaces supported by the
 *              host. Strings originate from an AML call to the _OSI method.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsValidateInterface (
    char                    *Interface)
{

    return (AE_SUPPORT);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsValidateAddress
 *
 * PARAMETERS:  SpaceId             - ACPI space ID
 *              Address             - Physical address
 *              Length              - Address length
 *
 * RETURN:      AE_OK if Address/Length is valid for the SpaceId. Otherwise,
 *              should return AE_AML_ILLEGAL_ADDRESS.
 *
 * DESCRIPTION: Validate a system address via the host OS. Used to validate
 *              the addresses accessed by AML operation regions.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsValidateAddress (
    UINT8                   SpaceId,
    ACPI_PHYSICAL_ADDRESS   Address,
    ACPI_SIZE               Length)
{

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadPciConfiguration
 *
 * PARAMETERS:  PciId               Seg/Bus/Dev
 *              Register            Device Register
 *              Value               Buffer where value is placed
 *              Width               Number of bits
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Read data from PCI configuration space
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsReadPciConfiguration (
    ACPI_PCI_ID             *PciId,
    UINT32                  Register,
    void                    *Value,
    UINT32                  Width)
{

    return (AE_ERROR);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsWritePciConfiguration
 *
 * PARAMETERS:  PciId               Seg/Bus/Dev
 *              Register            Device Register
 *              Value               Value to be written
 *              Width               Number of bits
 *
 * RETURN:      Status.
 *
 * DESCRIPTION: Write data to PCI configuration space
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWritePciConfiguration (
    ACPI_PCI_ID             *PciId,
    UINT32                  Register,
    ACPI_INTEGER            Value,
    UINT32                  Width)
{

    return (AE_ERROR);
}

/* TEMPORARY STUB FUNCTION */
void
AcpiOsDerivePciId(
    ACPI_HANDLE             rhandle,
    ACPI_HANDLE             chandle,
    ACPI_PCI_ID             **PciId)
{

}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadPort
 *
 * PARAMETERS:  Address             Address of I/O port/register to read
 *              Value               Where value is placed
 *              Width               Number of bits
 *
 * RETURN:      Value read from port
 *
 * DESCRIPTION: Read data from an I/O port or register
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsReadPort (
    ACPI_IO_ADDRESS         Address,
    UINT32                  *Value,
    UINT32                  Width)
{

    switch (Width)
    {
    case 8:
		*Value = _inp( Address );
        break;

    case 16:
        *Value = _inpw( Address );
        break;

    case 32:
        *Value = _inpd( Address );
        break;
    }

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsWritePort
 *
 * PARAMETERS:  Address             Address of I/O port/register to write
 *              Value               Value to write
 *              Width               Number of bits
 *
 * RETURN:      None
 *
 * DESCRIPTION: Write data to an I/O port or register
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWritePort (
    ACPI_IO_ADDRESS         Address,
    UINT32                  Value,
    UINT32                  Width)
{
	switch (Width)
    {
    case 8:
		_outp(Address, Value);
        break;

    case 16:
        _outpw(Address, Value);
        break;

    case 32:
        _outpd(Address, Value);
        break;
    }
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadMemory
 *
 * PARAMETERS:  Address             Physical Memory Address to read
 *              Value               Where value is placed
 *              Width               Number of bits
 *
 * RETURN:      Value read from physical memory address
 *
 * DESCRIPTION: Read data from a physical memory address
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsReadMemory (
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT32                  *Value,
    UINT32                  Width)
{

    switch (Width)
    {
    case 8:
    case 16:
    case 32:
        *Value = 0;
        break;

    default:
        return (AE_BAD_PARAMETER);
        break;
    }
    return (AE_ERROR);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsWriteMemory
 *
 * PARAMETERS:  Address             Physical Memory Address to write
 *              Value               Value to write
 *              Width               Number of bits
 *
 * RETURN:      None
 *
 * DESCRIPTION: Write data to a physical memory address
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWriteMemory (
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT32                  Value,
    UINT32                  Width)
{

    return (AE_ERROR);
}


ACPI_THREAD_ID
AcpiOsGetThreadId(void)
{
    return 1;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsSignal
 *
 * PARAMETERS:  Function            ACPI CA signal function code
 *              Info                Pointer to function-dependent structure
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Miscellaneous functions
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsSignal (
    UINT32                  Function,
    void                    *Info)
{

    switch (Function)
    {
    case ACPI_SIGNAL_FATAL:
        break;

    case ACPI_SIGNAL_BREAKPOINT:

        if (Info)
        {
            AcpiOsPrintf ("AcpiOsBreakpoint: %s ****\n", Info);
        }
        else
        {
            AcpiOsPrintf ("At AcpiOsBreakpoint ****\n");
        }

        break;
    }


    return (AE_OK);
}



