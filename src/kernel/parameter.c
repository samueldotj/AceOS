/*!
  \file		parameter.c
  \author	Samuel
  \version 	1.0
  \date	
  			Created: 28-Apr-2008 14:49
  			Last modified: 28-Apr-2008 14:49
  \brief	Kernel Parameter module
*/

#include <stdlib.h>
#include <string.h>
#include <ds/sort.h>
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/parameter.h>

char * sys_kernel_cmd_line = NULL;

static COMPARISION_RESULT compare_kernel_parameter(char * data1, char * data2);
static KERNEL_PARAMETER_PTR FindKernelParameter(char * parameter_name);
/*global static array*/
static KERNEL_PARAMETER kernel_parameters[] = {
	{"gdb_port", &sys_gdb_port, UINT32Validator, {0, 0xFFFF, 0}, UINT32Assignor, NULL}
};

void InitKernelParameters()
{
	KERNEL_PARAMETER temp;
	SortArray( (char *)kernel_parameters, (char *)&temp, sizeof(KERNEL_PARAMETER), 
					sizeof(kernel_parameters)/sizeof(KERNEL_PARAMETER),  compare_kernel_parameter );
				
}

void ParaseBootParameters()
{
	unsigned int argc, i;
	if ( sys_kernel_cmd_line == NULL )
		return;

	argc = str_total_tokens(sys_kernel_cmd_line, ' ');
	for(i=1; i<argc; i++)
	{
		char parameter[KERNEL_PARAMETER_NAME_MAX], value[KERNEL_PARAMETER_VALUE_MAX];
		char * parameter_name = parameter;
		unsigned int arg_len;
		/*parase the parameter name*/
		if( !str_get_token_info(sys_kernel_cmd_line, i, ' ', &arg_len) || arg_len >  KERNEL_PARAMETER_NAME_MAX )
		{
			kprintf("Error parsing boot command line.\n");
			continue;
		}
		str_get_token(parameter_name, sys_kernel_cmd_line, i, ' ');
		str_atrim( parameter_name );
		/*skip the prefix / or -*/
		if (parameter_name[0] == '/' || parameter_name[0] == '-' )
			parameter_name++;
		
		if ( FindKernelParameter( parameter_name ) == NULL )
		{
			kprintf("Unregonized kernel parameter \"%s\". skipping..\n", parameter_name);
			continue;
		}
		else
		{
			/*parse the value*/
			if ( i+1 < argc )
			{
				if( !str_get_token_info(sys_kernel_cmd_line, i, ' ', &arg_len) || arg_len >  KERNEL_PARAMETER_VALUE_MAX )
				{
					kprintf("Error parsing boot command line.\n");
					continue;
				}
				i++;
				str_get_token(value, sys_kernel_cmd_line, i, ' ');
				str_atrim( value );
				SetKernelVariable(parameter_name, value);
			}
		}
	}
}

int SetKernelVariable(char * parameter_name, char * value)
{
	KERNEL_PARAMETER_PTR kp;
	kp = FindKernelParameter(parameter_name);
	if ( kp == NULL )
		return NULL;
	
	/*validate the new value if we have to*/
	if ( kp->ValidateParameter )
	{
		if ( kp->ValidateParameter(kp, value) == -1 )
			return -1;
	}
	/*start assigning the new value*/
	if ( kp->AssignParameterStart(kp, value) == -1 )
		return -1;
	
	/*notify the subsystem assignment is complete*/
	if ( kp->AssignParameterComplete )
		kp->AssignParameterComplete(kp);
	
	/*success*/
	return 0;
}

void * GetKernelVariable(char * parameter_name)
{
	KERNEL_PARAMETER_PTR kp;
	kp = FindKernelParameter(parameter_name);
	if ( kp == NULL )
		return NULL;
	
	return kp->variable_address;
}

static KERNEL_PARAMETER_PTR FindKernelParameter(char * parameter_name)
{
	int i;
	KERNEL_PARAMETER_PTR result = kernel_parameters;
	for(i=0; i<sizeof(kernel_parameters)/sizeof(KERNEL_PARAMETER);i++ )
	{
		if ( strcmp(result->parameter_name, parameter_name) == 0 )
			return result;
		result++;
	}
	return NULL;
}

int UINT32Validator(KERNEL_PARAMETER_PTR kp, char * new_value)
{
	UINT32 val = strtoul(new_value, NULL, 0);
	if ( val < kp->ValidateParameterArg[0] || val > kp->ValidateParameterArg[1] )
		return -1;
	else
		return 0;
}

int UINT32Assignor(KERNEL_PARAMETER_PTR kp, char * new_value)
{
	*((UINT32 *)kp->variable_address) = strtoul(new_value, NULL, 0);;
	return 0;
}

static COMPARISION_RESULT compare_kernel_parameter(char * data1, char * data2)
{
	KERNEL_PARAMETER_PTR kp1, kp2;
	kp1 = (KERNEL_PARAMETER_PTR)data1;
	kp2 = (KERNEL_PARAMETER_PTR)data1;
	return strcmp( kp1->parameter_name, kp2->parameter_name );
}
