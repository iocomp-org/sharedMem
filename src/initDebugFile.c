#include <stdio.h>
#include <stdlib.h>  
#include <string.h> 
#include <errno.h>
#include "sharedmem.h"

#ifndef NDEBUG
void initDebugFile(struct params* ioParams,int globalRank)
{
	strcpy(ioParams->debugFile,"DEBUG_Rank_"); 
	
	// attach rank
	char stringRank[5]; 
	sprintf(stringRank,  "%d", globalRank); 
	strcat(ioParams->debugFile, stringRank); 
	
	// attach extension 
	strcat(ioParams->debugFile, ".out"); 

	remove(ioParams->debugFile);

	// create file in append mode 
	ioParams->debug = fopen(ioParams->debugFile,"a");
	if(ioParams->debug == NULL)
	{
		error_check(EXIT_FAILURE); 
		printf("Value of errno: %d\n", errno);
	}
} 
#endif 
