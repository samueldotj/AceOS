/*!
	\file	kernel/iom/driver.c
	\brief	driver loand and unloading routines.
*/
#include <ace.h>
#include <string.h>
#include <ctype.h>
#include <kernel/debug.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/elf.h>
#include <kernel/pm/task.h>
#include <kernel/iom/iom.h>
#include <kernel/vfs/vfs.h>

extern LIST_PTR driver_list_head;

ERROR_CODE FindDriverFile(char * device_id, char * buffer, int buf_length);

/*! Loads a driver*/
DRIVER_OBJECT_PTR LoadDriver(char * device_id)
{
	char driver_file_name[MAX_FILE_NAME], driver_file_path[MAX_FILE_PATH]="/boot/drivers/";
	DRIVER_OBJECT_PTR driver_object;
	ERROR_CODE (*DriverEntry)(DRIVER_OBJECT_PTR pDriverObject);
	ERROR_CODE err;
	LIST_PTR node;
	
	err = FindDriverFile(device_id, driver_file_name, sizeof(driver_file_name));
	ktrace("Driver for id %s : ", device_id);
	if ( err == ERROR_SUCCESS )
		ktrace("%s\n", driver_file_name);
	else
	{
		ktrace("%s\n", ERROR_CODE_AS_STRING(err) );
		return NULL;
	}		

	/*check whether the driver is already loaded*/
	LIST_FOR_EACH(node, driver_list_head)
	{
		driver_object = STRUCT_ADDRESS_FROM_MEMBER( node, DRIVER_OBJECT, driver_list );
		if ( strcmp( driver_file_name, driver_object->driver_file_name )==0 )
		{
			ktrace("Driver already loaded %s\n", driver_object->driver_file_name);
			return driver_object;
		}
			
	}
	strcat( driver_file_path, driver_file_name );
	kprintf("Loading %s: ", driver_file_path);
	
	err = LoadElfImage( driver_file_path, "DriverEntry", (void *)&DriverEntry );
	if ( err != ERROR_SUCCESS || DriverEntry == NULL )
		goto error;
	driver_object = AllocateBuffer( &driver_object_cache, CACHE_ALLOC_SLEEP );
	if ( driver_object == NULL )
	{
		err = ERROR_NOT_ENOUGH_MEMORY;
		goto error;
	}
	
	strcpy( driver_object->driver_file_name, driver_file_name);
	err = DriverEntry( driver_object );
	if ( err != ERROR_SUCCESS )
		goto error;

	/*add the driver to the driver list*/
	AddToList( driver_list_head, &driver_object->driver_list );
	kprintf("success\n");
	return driver_object;

error:
	kprintf("%s\n", ERROR_CODE_AS_STRING(err) );
	return NULL;

}

#define SKIP_WHITE_SPACES	while( i<file_size && isspace(va[i]) ) i++;
							

/*! Finds suitable driver for the given id and returns its full path in the given buffer
	\param device_id - device identification string
	\param buffer - buffer to place the driver path
	\param buf_length - buffer size
*/
ERROR_CODE FindDriverFile(char * device_id, char * buffer, int buf_length)
{
	ERROR_CODE err;
	int file_id,i=0;
	long file_size;
	char driver_id_database[] = "/boot/driver_id.txt";
	char * va;
	
	assert(device_id != NULL );
	assert(buffer != NULL );
	assert(buf_length > 0);
	
	buffer[0]=0;
	
	err = OpenFile( &kernel_task, driver_id_database, VFS_ACCESS_TYPE_READ, OPEN_EXISTING, &file_id);
	if ( err != ERROR_SUCCESS )
		goto done;
		
	err = GetFileSize(&kernel_task, file_id, &file_size);
	if ( err != ERROR_SUCCESS )
		goto done;

	err = MapViewOfFile(file_id, (VADDR *) &va, PROT_READ, 0, file_size, 0, 0);
	if ( err != ERROR_SUCCESS )
		goto done;
	
	err=ERROR_NOT_FOUND;
	/*read the file and try to find the driver file name*/
	while(i<file_size)
	{
		char driver_device_id[100];
		SKIP_WHITE_SPACES;

		/*if the line not starting with comment character process it*/
		if( va[i]!='#' )
		{
			int j=0;
			
			/*copy the driver id*/
			while( i<file_size && j<sizeof(driver_device_id)-1 && !isspace(va[i]) )
				driver_device_id[j++] = va[i++];
			driver_device_id[j]=0;
			
			/*if user didnt provide driver file name break*/
			if ( va[i] != ' ' && va[i] != '\t' )
				break;
				
			SKIP_WHITE_SPACES;
			
			if( strcmp(driver_device_id, device_id) == 0 )
			{
				/*copy driver file name*/
				j=0;
				while( i<file_size && j<buf_length-1 && !isspace(va[i]) )
					buffer[j++] = va[i++];
					
				buffer[j]=0;
				err = ERROR_SUCCESS;
				break;
			}
		}
		/*skip till end of line*/
		while( i<file_size && va[i]!='\n') i++;
	}
	
done:
	/* \todo - release the mapping */
	
	CloseFile(&kernel_task, file_id);
	return err;
}
