#include <stdio.h>
#include <stdlib.h>  
#include <string.h> 
#include "stream_post_ioserver.h"
// each window can write to its own file, initialise write file name for
// each window number 

void fileNameInit(struct params* ioParams, int windowNum)
{
		int arrayNumInt = '0' + windowNum; 
		char arrayNumChar = (char) arrayNumInt; 
		char arrayNumString[] = {arrayNumChar, '\0'}; 
		strcpy(ioParams->WRITEFILE[windowNum], arrayNumString); 
		char EXT[] = ".dat"; 
		// char EXT[] = "array.h5"; 
		strcat(ioParams->WRITEFILE[windowNum], EXT); 

		// delete the previous files 
		int test = remove(ioParams->WRITEFILE[windowNum]);
} 
