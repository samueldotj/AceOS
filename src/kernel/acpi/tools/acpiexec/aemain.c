/******************************************************************************
 *
 * Module Name: aemain - Main routine for the AcpiExec utility
 *              $Revision: 1.112 $
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

#include "aecommon.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#define _COMPONENT          PARSER
        ACPI_MODULE_NAME    ("aemain")

UINT8           AcpiGbl_BatchMode = 0;
BOOLEAN         AcpiGbl_IgnoreErrors = FALSE;
char            BatchBuffer[128];


/******************************************************************************
 *
 * FUNCTION:    usage
 *
 * PARAMETERS:  None
 *
 * RETURN:      None
 *
 * DESCRIPTION: Print a usage message
 *
 *****************************************************************************/

static void
usage (void)
{
    printf ("Usage: acpiexec [Options] [InputFile]\n\n");

    printf ("Where:\n");
    printf ("   -?                  Display this message\n");
    printf ("   -a                  Do not abort methods on error\n");
    printf ("   -b <CommandLine>    Batch mode command execution\n");
    printf ("   -e [Method]         Batch mode method execution\n");
    printf ("   -i                  Do not run STA/INI methods during init\n");
    printf ("   -m                  Display final memory use statistics\n");
    printf ("   -o <OutputFile>     Send output to this file\n");
    printf ("   -s                  Enable Interpreter Slack Mode\n");
    printf ("   -t                  Enable Interpreter Serialized Mode\n");
    printf ("   -v                  Verbose init output\n");
    printf ("   -x <DebugLevel>     Specify debug output level\n");
}


/******************************************************************************
 *
 * FUNCTION:    AcpiDbRunBatchMode
 *
 * PARAMETERS:  BatchCommandLine    - A semicolon separated list of commands
 *                                    to be executed.
 *                                    Use only commas to separate elements of
 *                                    particular command.
 * RETURN:      Status
 *
 * DESCRIPTION: For each command of list separated by ';' prepare the command
 *              buffer and pass it to AcpiDbCommandDispatch.
 *
 *****************************************************************************/

static ACPI_STATUS
AcpiDbRunBatchMode (
    void)
{
    ACPI_STATUS             Status;
    char                    *Ptr = BatchBuffer;
    char                    *Cmd = Ptr;
    UINT8                   Run = 0;

    AcpiGbl_MethodExecuting = FALSE;
    AcpiGbl_StepToNextCall = FALSE;

    while (*Ptr)
    {
        if (*Ptr == ',')
        {
            /* Convert commas to spaces */
            *Ptr = ' ';
        }
        else if (*Ptr == ';')
        {
            *Ptr = '\0';
            Run = 1;
        }

        Ptr++;

        if (Run || (*Ptr == '\0'))
        {
            (void) AcpiDbCommandDispatch (Cmd, NULL, NULL);
            Run = 0;
            Cmd = Ptr;
        }
    }

    Status = AcpiTerminate ();
    return (Status);
}


/******************************************************************************
 *
 * FUNCTION:    main
 *
 * PARAMETERS:  argc, argv
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Main routine for AcpiDump utility
 *
 *****************************************************************************/

int ACPI_SYSTEM_XFACE
main (
    int                     argc,
    char                    **argv)
{
    int                     j;
    ACPI_STATUS             Status;
    UINT32                  InitFlags;
    ACPI_TABLE_HEADER       *Table;


#ifdef _DEBUG
    _CrtSetDbgFlag (_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF |
                    _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
#endif

    printf ("\nIntel ACPI Component Architecture\nAML Execution/Debug Utility");
    printf (" version %8.8X", ((UINT32) ACPI_CA_VERSION));
    printf (" [%s]\n\n",  __DATE__);

    if (argc < 2)
    {
        usage ();
        return 0;
    }

    signal (SIGINT, AeCtrlCHandler);

    /* Init globals */

    AcpiDbgLevel = ACPI_NORMAL_DEFAULT;
    AcpiDbgLayer = 0xFFFFFFFF;

    /* Init ACPI and start debugger thread */

    AcpiInitializeSubsystem ();
    AcpiGbl_GlobalLockPresent = TRUE;

    /* Get the command line options */

    while ((j = AcpiGetopt (argc, argv, "?ab:de^gimo:stvx:")) != EOF) switch(j)
    {
    case 'a':
        AcpiGbl_IgnoreErrors = TRUE;
        break;

    case 'b':
        if (strlen (AcpiGbl_Optarg) > 127)
        {
            printf ("**** The length of command line (%u) exceeded maximum (127)\n",
                strlen (AcpiGbl_Optarg));
            return -1;
        }
        AcpiGbl_BatchMode = 1;
        strcpy (BatchBuffer, AcpiGbl_Optarg);
        break;

    case 'd':
        AcpiGbl_DbOpt_disasm = TRUE;
        AcpiGbl_DbOpt_stats = TRUE;
        break;

    case 'e':
        AcpiGbl_BatchMode = 2;
        switch (AcpiGbl_Optarg[0])
        {
        case '^':
            strcpy (BatchBuffer, "MAIN");
            break;
        default:
            strcpy (BatchBuffer, AcpiGbl_Optarg);
            break;
        }
        break;

    case 'g':
        AcpiGbl_DbOpt_tables = TRUE;
        AcpiGbl_DbFilename = NULL;
        break;

    case 'i':
        AcpiGbl_DbOpt_ini_methods = FALSE;
        break;

    case 'm':
#ifdef ACPI_DBG_TRACK_ALLOCATIONS
        AcpiGbl_DisplayFinalMemStats = TRUE;
#endif
        break;

    case 'o':
        printf ("O option is not implemented\n");
        break;

    case 's':
        AcpiGbl_EnableInterpreterSlack = TRUE;
        printf ("Enabling AML Interpreter slack mode\n");
        break;

    case 't':
        AcpiGbl_AllMethodsSerialized = TRUE;
        printf ("Enabling AML Interpreter serialized mode\n");
        break;

    case 'v':
        AcpiDbgLevel |= ACPI_LV_INIT_NAMES;
        break;

    case 'x':
        AcpiDbgLevel = strtoul (AcpiGbl_Optarg, NULL, 0);
        AcpiGbl_DbConsoleDebugLevel = AcpiDbgLevel;
        printf ("Debug Level: 0x%8.8X\n", AcpiDbgLevel);
        break;

    case '?':
    default:
        usage();
        return -1;
    }


    InitFlags = (ACPI_NO_HANDLER_INIT | ACPI_NO_ACPI_ENABLE);
    if (!AcpiGbl_DbOpt_ini_methods)
    {
        InitFlags |= (ACPI_NO_DEVICE_INIT | ACPI_NO_OBJECT_INIT);
    }

    /* Standalone filename is the only argument */

    if (argv[AcpiGbl_Optind])
    {
        AcpiGbl_DbOpt_tables = TRUE;
        AcpiGbl_DbFilename = argv[AcpiGbl_Optind];

        Status = AcpiDbReadTableFromFile (AcpiGbl_DbFilename, &Table);
        if (ACPI_FAILURE (Status))
        {
            printf ("**** Could not get input table, %s\n", AcpiFormatException (Status));
            goto enterloop;
        }

        AeBuildLocalTables (Table);
        Status = AeInstallTables ();
        if (ACPI_FAILURE (Status))
        {
            printf ("**** Could not load ACPI tables, %s\n", AcpiFormatException (Status));
            goto enterloop;
        }

        Status = AeInstallHandlers ();
        if (ACPI_FAILURE (Status))
        {
            goto enterloop;
        }

        /*
         * TBD:
         * Need a way to call this after the "LOAD" command
         */
        Status = AcpiEnableSubsystem (InitFlags);
        if (ACPI_FAILURE (Status))
        {
            printf ("**** Could not EnableSubsystem, %s\n", AcpiFormatException (Status));
            goto enterloop;
        }

        Status = AcpiInitializeObjects (InitFlags);
        if (ACPI_FAILURE (Status))
        {
            printf ("**** Could not InitializeObjects, %s\n", AcpiFormatException (Status));
            goto enterloop;
        }

        AeMiscellaneousTests ();
    }

enterloop:

    if (AcpiGbl_BatchMode == 1)
    {
        AcpiDbRunBatchMode ();
    }
    else if (AcpiGbl_BatchMode == 2)
    {
        AcpiDbExecute (BatchBuffer, NULL, EX_NO_SINGLE_STEP);
    }
    else
    {
        /* Enter the debugger command loop */

        AcpiDbUserCommands (ACPI_DEBUGGER_COMMAND_PROMPT, NULL);
    }

    return 0;
}
