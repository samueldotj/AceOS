/*!
	\file		parameter.h
	\author		Samuel(samueldotj@gmail.com)
	\version		1.0
	\date	
				Created: 28-Apr-08 14:48
				Last modified: 
*/

#ifndef __PARAMETER__H
#define __PARAMETER__H

#include <ace.h>

extern char * sys_kernel_cmd_line;

#define KERNEL_PARAMETER_NAME_MAX	30
#define	KERNEL_PARAMETER_VALUE_MAX	50

typedef struct kernel_parameter KERNEL_PARAMETER, * KERNEL_PARAMETER_PTR;

struct kernel_parameter
{
	char parameter_name[KERNEL_PARAMETER_NAME_MAX];
	void * variable_address;
	
	int (*ValidateParameter)(KERNEL_PARAMETER_PTR kp, char * new_value);
	UINT32 ValidateParameterArg[3];

	int (*AssignParameterStart)(KERNEL_PARAMETER_PTR kp, char * new_value);
	int (*AssignParameterComplete)(KERNEL_PARAMETER_PTR kp);
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
