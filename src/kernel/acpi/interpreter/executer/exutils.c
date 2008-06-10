
/******************************************************************************
 *
 * Module Name: exutils - interpreter/scanner utilities
 *              $Revision: 1.131 $
 *
 *****************************************************************************/

/******************************************************************************
 *
 * 1. Copyright Notice
 *
 * Some or all of this work - Copyright (c) 1999 - 2008, Intel Corp.
 * All rights reserved.
 *
 * 2. License
 *
 * 2.1. This is your license from Intel Corp. under its intellectual property
 * rights.  You may have additional license terms from the party that provided
 * you this software, covering your right to use that party's intellectual
 * property rights.
 *
 * 2.2. Intel grants, free of charge, to any person ("Licensee") obtaining a
 * copy of the source code appearing in this file ("Covered Code") an
 * irrevocable, perpetual, worldwide license under Intel's copyrights in the
 * base code distributed originally by Intel ("Original Intel Code") to copy,
 * make derivatives, distribute, use and display any portion of the Covered
 * Code in any form, with the right to sublicense such rights; and
 *
 * 2.3. Intel grants Licensee a non-exclusive and non-transferable patent
 * license (with the right to sublicense), under only those claims of Intel
 * patents that are infringed by the Original Intel Code, to make, use, sell,
 * offer to sell, and import the Covered Code and derivative works thereof
 * solely to the minimum extent necessary to exercise the above copyright
 * license, and in no event shall the patent license extend to any additions
 * to or modifications of the Original Intel Code.  No other license or right
 * is granted directly or by implication, estoppel or otherwise;
 *
 * The above copyright and patent license is granted only if the following
 * conditions are met:
 *
 * 3. Conditions
 *
 * 3.1. Redistribution of Source with Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification with rights to further distribute source must include
 * the above Copyright Notice, the above License, this list of Conditions,
 * and the following Disclaimer and Export Compliance provision.  In addition,
 * Licensee must cause all Covered Code to which Licensee contributes to
 * contain a file documenting the changes Licensee made to create that Covered
 * Code and the date of any change.  Licensee must include in that file the
 * documentation of any changes made by any predecessor Licensee.  Licensee
 * must include a prominent statement that the modification is derived,
 * directly or indirectly, from Original Intel Code.
 *
 * 3.2. Redistribution of Source with no Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification without rights to further distribute source must
 * include the following Disclaimer and Export Compliance provision in the
 * documentation and/or other materials provided with distribution.  In
 * addition, Licensee may not authorize further sublicense of source of any
 * portion of the Covered Code, and must include terms to the effect that the
 * license from Licensee to its licensee is limited to the intellectual
 * property embodied in the software Licensee provides to its licensee, and
 * not to intellectual property embodied in modifications its licensee may
 * make.
 *
 * 3.3. Redistribution of Executable. Redistribution in executable form of any
 * substantial portion of the Covered Code or modification must reproduce the
 * above Copyright Notice, and the following Disclaimer and Export Compliance
 * provision in the documentation and/or other materials provided with the
 * distribution.
 *
 * 3.4. Intel retains all right, title, and interest in and to the Original
 * Intel Code.
 *
 * 3.5. Neither the name Intel nor any other trademark owned or controlled by
 * Intel shall be used in advertising or otherwise to promote the sale, use or
 * other dealings in products derived from or relating to the Covered Code
 * without prior written authorization from Intel.
 *
 * 4. Disclaimer and Export Compliance
 *
 * 4.1. INTEL MAKES NO WARRANTY OF ANY KIND REGARDING ANY SOFTWARE PROVIDED
 * HERE.  ANY SOFTWARE ORIGINATING FROM INTEL OR DERIVED FROM INTEL SOFTWARE
 * IS PROVIDED "AS IS," AND INTEL WILL NOT PROVIDE ANY SUPPORT,  ASSISTANCE,
 * INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL WILL NOT PROVIDE ANY
 * UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY DISCLAIMS ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGEMENT AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * 4.2. IN NO EVENT SHALL INTEL HAVE ANY LIABILITY TO LICENSEE, ITS LICENSEES
 * OR ANY OTHER THIRD PARTY, FOR ANY LOST PROFITS, LOST DATA, LOSS OF USE OR
 * COSTS OF PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, OR FOR ANY INDIRECT,
 * SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THIS AGREEMENT, UNDER ANY
 * CAUSE OF ACTION OR THEORY OF LIABILITY, AND IRRESPECTIVE OF WHETHER INTEL
 * HAS ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES.  THESE LIMITATIONS
 * SHALL APPLY NOTWITHSTANDING THE FAILURE OF THE ESSENTIAL PURPOSE OF ANY
 * LIMITED REMEDY.
 *
 * 4.3. Licensee shall not export, either directly or indirectly, any of this
 * software or system incorporating such software without first obtaining any
 * required license or other approval from the U. S. Department of Commerce or
 * any other agency or department of the United States Government.  In the
 * event Licensee exports any such software from the United States or
 * re-exports any such software from a foreign destination, Licensee shall
 * ensure that the distribution and export/re-export of the software is in
 * compliance with all laws, regulations, orders, or other restrictions of the
 * U.S. Export Administration Regulations. Licensee agrees that neither it nor
 * any of its subsidiaries will export/re-export any technical data, process,
 * software, or service, directly or indirectly, to any country for which the
 * United States government or any agency thereof requires an export license,
 * other governmental approval, or letter of assurance, without first obtaining
 * such license, approval or letter.
 *
 *****************************************************************************/

#define __EXUTILS_C__

/*
 * DEFINE_AML_GLOBALS is tested in amlcode.h
 * to determine whether certain global names should be "defined" or only
 * "declared" in the current compilation.  This enhances maintainability
 * by enabling a single header file to embody all knowledge of the names
 * in question.
 *
 * Exactly one module of any executable should #define DEFINE_GLOBALS
 * before #including the header files which use this convention.  The
 * names in question will be defined and initialized in that module,
 * and declared as extern in all other modules which #include those
 * header files.
 */

#define DEFINE_AML_GLOBALS

#include "acpi.h"
#include "acinterp.h"
#include "amlcode.h"

#define _COMPONENT          ACPI_EXECUTER
        ACPI_MODULE_NAME    ("exutils")

/* Local prototypes */

static UINT32
AcpiExDigitsNeeded (
    ACPI_INTEGER            Value,
    UINT32                  Base);


#ifndef ACPI_NO_METHOD_EXECUTION
/*******************************************************************************
 *
 * FUNCTION:    AcpiExEnterInterpreter
 *
 * PARAMETERS:  None
 *
 * RETURN:      None
 *
 * DESCRIPTION: Enter the interpreter execution region. Failure to enter
 *              the interpreter region is a fatal system error. Used in
 *              conjunction with ExitInterpreter.
 *
 ******************************************************************************/

void
AcpiExEnterInterpreter (
    void)
{
    ACPI_STATUS             Status;


    ACPI_FUNCTION_TRACE (ExEnterInterpreter);


    Status = AcpiUtAcquireMutex (ACPI_MTX_INTERPRETER);
    if (ACPI_FAILURE (Status))
    {
        ACPI_ERROR ((AE_INFO, "Could not acquire AML Interpreter mutex"));
    }

    return_VOID;
}


/*******************************************************************************
 *
 * FUNCTION:    AcpiExReacquireInterpreter
 *
 * PARAMETERS:  None
 *
 * RETURN:      None
 *
 * DESCRIPTION: Reacquire the interpreter execution region from within the
 *              interpreter code. Failure to enter the interpreter region is a
 *              fatal system error. Used in  conjuction with
 *              RelinquishInterpreter
 *
 ******************************************************************************/

void
AcpiExReacquireInterpreter (
    void)
{
    ACPI_FUNCTION_TRACE (ExReacquireInterpreter);


    /*
     * If the global serialized flag is set, do not release the interpreter,
     * since it was not actually released by AcpiExRelinquishInterpreter.
     * This forces the interpreter to be single threaded.
     */
    if (!AcpiGbl_AllMethodsSerialized)
    {
        AcpiExEnterInterpreter ();
    }

    return_VOID;
}


/*******************************************************************************
 *
 * FUNCTION:    AcpiExExitInterpreter
 *
 * PARAMETERS:  None
 *
 * RETURN:      None
 *
 * DESCRIPTION: Exit the interpreter execution region. This is the top level
 *              routine used to exit the interpreter when all processing has
 *              been completed.
 *
 ******************************************************************************/

void
AcpiExExitInterpreter (
    void)
{
    ACPI_STATUS             Status;


    ACPI_FUNCTION_TRACE (ExExitInterpreter);


    Status = AcpiUtReleaseMutex (ACPI_MTX_INTERPRETER);
    if (ACPI_FAILURE (Status))
    {
        ACPI_ERROR ((AE_INFO, "Could not release AML Interpreter mutex"));
    }

    return_VOID;
}


/*******************************************************************************
 *
 * FUNCTION:    AcpiExRelinquishInterpreter
 *
 * PARAMETERS:  None
 *
 * RETURN:      None
 *
 * DESCRIPTION: Exit the interpreter execution region, from within the
 *              interpreter - before attempting an operation that will possibly
 *              block the running thread.
 *
 * Cases where the interpreter is unlocked internally
 *      1) Method to be blocked on a Sleep() AML opcode
 *      2) Method to be blocked on an Acquire() AML opcode
 *      3) Method to be blocked on a Wait() AML opcode
 *      4) Method to be blocked to acquire the global lock
 *      5) Method to be blocked waiting to execute a serialized control method
 *          that is currently executing
 *      6) About to invoke a user-installed opregion handler
 *
 ******************************************************************************/

void
AcpiExRelinquishInterpreter (
    void)
{
    ACPI_FUNCTION_TRACE (ExRelinquishInterpreter);


    /*
     * If the global serialized flag is set, do not release the interpreter.
     * This forces the interpreter to be single threaded.
     */
    if (!AcpiGbl_AllMethodsSerialized)
    {
        AcpiExExitInterpreter ();
    }

    return_VOID;
}


/*******************************************************************************
 *
 * FUNCTION:    AcpiExTruncateFor32bitTable
 *
 * PARAMETERS:  ObjDesc         - Object to be truncated
 *
 * RETURN:      none
 *
 * DESCRIPTION: Truncate an ACPI Integer to 32 bits if the execution mode is
 *              32-bit, as determined by the revision of the DSDT.
 *
 ******************************************************************************/

void
AcpiExTruncateFor32bitTable (
    ACPI_OPERAND_OBJECT     *ObjDesc)
{

    ACPI_FUNCTION_ENTRY ();


    /*
     * Object must be a valid number and we must be executing
     * a control method. NS node could be there for AML_INT_NAMEPATH_OP.
     */
    if ((!ObjDesc) ||
        (ACPI_GET_DESCRIPTOR_TYPE (ObjDesc) != ACPI_DESC_TYPE_OPERAND) ||
        (ACPI_GET_OBJECT_TYPE (ObjDesc) != ACPI_TYPE_INTEGER))
    {
        return;
    }

    if (AcpiGbl_IntegerByteWidth == 4)
    {
        /*
         * We are running a method that exists in a 32-bit ACPI table.
         * Truncate the value to 32 bits by zeroing out the upper 32-bit field
         */
        ObjDesc->Integer.Value &= (ACPI_INTEGER) ACPI_UINT32_MAX;
    }
}


/*******************************************************************************
 *
 * FUNCTION:    AcpiExAcquireGlobalLock
 *
 * PARAMETERS:  FieldFlags            - Flags with Lock rule:
 *                                      AlwaysLock or NeverLock
 *
 * RETURN:      None
 *
 * DESCRIPTION: Obtain the ACPI hardware Global Lock, only if the field
 *              flags specifiy that it is to be obtained before field access.
 *
 ******************************************************************************/

void
AcpiExAcquireGlobalLock (
    UINT32                  FieldFlags)
{
    ACPI_STATUS             Status;


    ACPI_FUNCTION_TRACE (ExAcquireGlobalLock);


    /* Only use the lock if the AlwaysLock bit is set */

    if (!(FieldFlags & AML_FIELD_LOCK_RULE_MASK))
    {
        return_VOID;
    }

    /* Attempt to get the global lock, wait forever */

    Status = AcpiExAcquireMutexObject (ACPI_WAIT_FOREVER,
                AcpiGbl_GlobalLockMutex, AcpiOsGetThreadId ());

    if (ACPI_FAILURE (Status))
    {
        ACPI_EXCEPTION ((AE_INFO, Status,
            "Could not acquire Global Lock"));
    }

    return_VOID;
}


/*******************************************************************************
 *
 * FUNCTION:    AcpiExReleaseGlobalLock
 *
 * PARAMETERS:  FieldFlags            - Flags with Lock rule:
 *                                      AlwaysLock or NeverLock
 *
 * RETURN:      None
 *
 * DESCRIPTION: Release the ACPI hardware Global Lock
 *
 ******************************************************************************/

void
AcpiExReleaseGlobalLock (
    UINT32                  FieldFlags)
{
    ACPI_STATUS             Status;


    ACPI_FUNCTION_TRACE (ExReleaseGlobalLock);


    /* Only use the lock if the AlwaysLock bit is set */

    if (!(FieldFlags & AML_FIELD_LOCK_RULE_MASK))
    {
        return_VOID;
    }

    /* Release the global lock */

    Status = AcpiExReleaseMutexObject (AcpiGbl_GlobalLockMutex);
    if (ACPI_FAILURE (Status))
    {
        /* Report the error, but there isn't much else we can do */

        ACPI_EXCEPTION ((AE_INFO, Status,
            "Could not release Global Lock"));
    }

    return_VOID;
}


/*******************************************************************************
 *
 * FUNCTION:    AcpiExDigitsNeeded
 *
 * PARAMETERS:  Value           - Value to be represented
 *              Base            - Base of representation
 *
 * RETURN:      The number of digits.
 *
 * DESCRIPTION: Calculate the number of digits needed to represent the Value
 *              in the given Base (Radix)
 *
 ******************************************************************************/

static UINT32
AcpiExDigitsNeeded (
    ACPI_INTEGER            Value,
    UINT32                  Base)
{
    UINT32                  NumDigits;
    ACPI_INTEGER            CurrentValue;


    ACPI_FUNCTION_TRACE (ExDigitsNeeded);


    /* ACPI_INTEGER is unsigned, so we don't worry about a '-' prefix */

    if (Value == 0)
    {
        return_UINT32 (1);
    }

    CurrentValue = Value;
    NumDigits = 0;

    /* Count the digits in the requested base */

    while (CurrentValue)
    {
        (void) AcpiUtShortDivide (CurrentValue, Base, &CurrentValue, NULL);
        NumDigits++;
    }

    return_UINT32 (NumDigits);
}


/*******************************************************************************
 *
 * FUNCTION:    AcpiExEisaIdToString
 *
 * PARAMETERS:  NumericId       - EISA ID to be converted
 *              OutString       - Where to put the converted string (8 bytes)
 *
 * RETURN:      None
 *
 * DESCRIPTION: Convert a numeric EISA ID to string representation
 *
 ******************************************************************************/

void
AcpiExEisaIdToString (
    UINT32                  NumericId,
    char                    *OutString)
{
    UINT32                  EisaId;


    ACPI_FUNCTION_ENTRY ();


    /* Swap ID to big-endian to get contiguous bits */

    EisaId = AcpiUtDwordByteSwap (NumericId);

    OutString[0] = (char) ('@' + (((unsigned long) EisaId >> 26) & 0x1f));
    OutString[1] = (char) ('@' + ((EisaId >> 21) & 0x1f));
    OutString[2] = (char) ('@' + ((EisaId >> 16) & 0x1f));
    OutString[3] = AcpiUtHexToAsciiChar ((ACPI_INTEGER) EisaId, 12);
    OutString[4] = AcpiUtHexToAsciiChar ((ACPI_INTEGER) EisaId, 8);
    OutString[5] = AcpiUtHexToAsciiChar ((ACPI_INTEGER) EisaId, 4);
    OutString[6] = AcpiUtHexToAsciiChar ((ACPI_INTEGER) EisaId, 0);
    OutString[7] = 0;
}


/*******************************************************************************
 *
 * FUNCTION:    AcpiExUnsignedIntegerToString
 *
 * PARAMETERS:  Value           - Value to be converted
 *              OutString       - Where to put the converted string (8 bytes)
 *
 * RETURN:      None, string
 *
 * DESCRIPTION: Convert a number to string representation. Assumes string
 *              buffer is large enough to hold the string.
 *
 ******************************************************************************/

void
AcpiExUnsignedIntegerToString (
    ACPI_INTEGER            Value,
    char                    *OutString)
{
    UINT32                  Count;
    UINT32                  DigitsNeeded;
    UINT32                  Remainder;


    ACPI_FUNCTION_ENTRY ();


    DigitsNeeded = AcpiExDigitsNeeded (Value, 10);
    OutString[DigitsNeeded] = 0;

    for (Count = DigitsNeeded; Count > 0; Count--)
    {
        (void) AcpiUtShortDivide (Value, 10, &Value, &Remainder);
        OutString[Count-1] = (char) ('0' + Remainder);\
    }
}

#endif
