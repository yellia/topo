
#include "file_manager.h"

/**************************************************************************
*Description:
*		save string to filepath		
*Parameters:	 
*		filepath - save absolute path 	
*		s - save string 
*return value:
*		1 success or -1 failed	
***************************************************************************/

int file_save(const char * filepath,const char *s)
{
	FILE * pf = fopen(filepath, "w");
	int ret = -1;

	if(pf != NULL)
	{
		fprintf(pf, "%s",s);	
		
		fclose(pf);

		pf = NULL;	

		ret = 1;
	}

	return ret;
}

