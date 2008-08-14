/*!	\file	parameter.h
	\brief	kernel parameter
*/

#ifndef __PARAMETER__H
#define __PARAMETER__H

#include <ace.h>

extern char * sys_kernel_cmd_line;

/*! maxmium length of parameter name*/
#define KERNEL_PARAMETER_NAME_MAX	30
/*! maxmium length of parameter value*/
#define	KERNEL_PARAMETER_VALUE_MAX	50

typedef struct kernel_parameter KERNEL_PARAMETER, * KERNEL_PARAMETER_PTR;

struct kernel_parameter
{
	char parameter_name[KERNEL_PARAMETER_NAME_MAX];								/*! Name of the parameter*/
	void * variable_address;													/*! Which kernel variable this parameter affects*/
	
	int (*ValidateParameter)(KERNEL_PARAMETER_PTR kp, char * new_value);		/*! Function to validate this parameter*/
	UINT32 ValidateParameterArg[3];												/*! Arguments to the above function*/

	int (*AssignParameterStart)(KERNEL_PARAMETER_PTR kp, char * new_value);		/*! Function to assign the value to kernel variable*/
	int (*AssignParameterComplete)(KERNEL_PARAMETER_PTR kp);					/*! Notification function to call after assigning the value to kernel variable*/
};

#ifdef __cplusplus
	extern "C" {
#endif

void InitKernelParameters();
void ParaseBootParameters();

int SetKernelVariable(char * parameter_name, char * value);
void * GetKernelVariable(char * parameter_name);

int UINT32Validator(KERNEL_PARAMETER_PTR kp, char * new_value);
int UINT32Assignor(KERNEL_PARAMETER_PTR kp, char * new_value);

#ifdef __cplusplus
	}
#endif

#endif
