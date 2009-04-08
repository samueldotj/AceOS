/*
	http://www.gnu.org/software/tar/manual/html_node/Standard.html
	http://www.mkssoftware.com/docs/man4/tar.4.asp
*/
#include <ace.h>
#include <tar.h>
#include <string.h>
#include <ds/bits.h>
#include <ds/align.h>
#include <kernel/vfs/vfs.h>

typedef enum
{
	TAR_CALLBACK_RESULT_UNKNOWN,
	TAR_CALLBACK_RESULT_SUCCESS,
	TAR_CALLBACK_RESULT_NOT_FOUND
}TAR_CALLBACK_RESULT;

typedef struct
{
	char * 				file_name;		/*file_name to find*/
	long				inode;			/*inode to find*/
	
	FILE_STAT_PARAM_PTR buffer;			/*buffer to store file_stat_param*/
	int 				max_entries;	/*max entries to put in buffer*/
	int 				total_entries;	/*total entires in buffer*/
	
	TAR_CALLBACK_RESULT	result;			/*result is stored here*/
}TAR_CALLBACK_PARAM, * TAR_CALLBACK_PARAM_PTR;
	

#define isOctal(ch)     (((ch) >= '0') && ((ch) <= '7'))

/* Read an octal value in a field of the specified width, with optional  spaces on both sides of the number and with an optional null character at the end.  
	\param cp - string
	\param size - size of the string
	\return -1 on an illegal format.  
*/
static long getOctal(const char *cp, int size)
{
	long val = 0;

	for(;(size > 0) && (*cp == ' '); cp++, size--);
	if ((size == 0) || !isOctal(*cp))
		return -1;
	for(; (size > 0) && isOctal(*cp); size--) {
		val = val * 8 + *cp++ - '0';
	}
	for (;(size > 0) && (*cp == ' '); cp++, size--);
	if ((size > 0) && *cp)
		return -1;
	return val;
}

/*! Fills the buffer with passed tar directory entries
	\param tar_index - content index in the tar file
	\param tar_header - content header of the tar file
	\param file_start - where the contents starts
	\param file_size - size of the content file
	\param argument - see the strcuture definition
*/
static int FillTarDirectoryEntries(int tar_index, TAR_POSIX_HEADER_PTR tar_header, void * file_start, long file_size, TAR_CALLBACK_PARAM_PTR argument)
{
	if(    (argument->file_name==NULL && argument->inode == -1) || 			/*case 1 - list all directory entries - neither file name nor inode provided*/
			argument->inode == tar_index  || 								/*case 2 - find a directory entry for a particular inode */
			strcmp(argument->file_name, &tar_header->name[1])==0 			/*case 3 - find a directory entry for a particular file name */
		)
	{
		strcpy(argument->buffer->name, &tar_header->name[1]);
		argument->buffer->inode = tar_index;
		argument->buffer->fs_data = file_start;
		argument->buffer->file_size = file_size;	
		argument->buffer++;
		argument->total_entries++;
		/*break enumeration - filename found or max count reached*/
		if ( argument->file_name != NULL || argument->inode == tar_index || argument->total_entries >= argument->max_entries )
		{
			argument->result = TAR_CALLBACK_RESULT_SUCCESS;
			return 1;
		}
	}
	/*continue enumerating*/
	return 0;
}

/*! Enumerates files in a tar by calling the provided callback function
	\param tar_start - virtual address of tar file start
	\param callback - function to call when a matching entry found.
	\param argument - argument to pass to callback function.
	\return file count
*/
static int EnumerateFilesInTar(char * tar_start, int (*callback)(int tar_index, TAR_POSIX_HEADER_PTR tar_header, void * file_start, long file_size, TAR_CALLBACK_PARAM_PTR argument), void * callback_argument )
{
	int index=0;
	TAR_POSIX_HEADER_PTR header = (TAR_POSIX_HEADER_PTR)tar_start;
	
	while( header->name[0] != 0 )
	{
		long size = getOctal(header->size, sizeof(header->size));
		/*call the callback function if provided*/
		if ( callback && callback(index, header, ((char *)header)+sizeof(TAR_POSIX_HEADER), size, callback_argument) )
			return index;

		/*next entry*/
		index++;
		header = (TAR_POSIX_HEADER_PTR) ( ((char *)header) + sizeof(TAR_POSIX_HEADER) + ALIGN_UP(size, 9) );
	}
	return index;
}

/*! Returns number of directory entries in a tar file
	\param tar_start - Address of the content of tar file
	\return total number of files and directories in the tar
*/
int GetDirectoryEntryCountInTar(char * tar_start)
{
	return EnumerateFilesInTar(tar_start, NULL, NULL);
}
/*! Returns directory entries in a tar file
	\param tar_start - Address of the content of tar file
	\param file_name - if not null, only specified directory entry will be returned
	\param index - if given(>0) only that specified directory entry will be returnd
	\param buffer - the resulting directory entries will be placed in this buffer
	\param max_entries - size/count of the buffer	
	\return True on success
			False on failure
*/
int GetDirectoryEntriesInTar(char * tar_start, char * file_name, int inode, FILE_STAT_PARAM_PTR buffer, int max_entries)
{
	TAR_CALLBACK_PARAM param;
	param.buffer = buffer;
	param.max_entries = max_entries;
	param.file_name = file_name;
	param.inode = inode;
	param.total_entries = 0;
	param.result = TAR_CALLBACK_RESULT_UNKNOWN;
	
	EnumerateFilesInTar(tar_start, FillTarDirectoryEntries, &param);
	
	return ( param.result == TAR_CALLBACK_RESULT_SUCCESS );
}

