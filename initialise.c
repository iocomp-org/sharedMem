#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h> 
#include <string.h> 
#include <assert.h>
#include "stream_post_ioserver.h"

void initialise(int argc, char** argv, struct params *ioParams)
{
	for(int i = 0; i < argc; i++)
	{
		if(strcmp(argv[i],"--N") == 0)
		{
			ioParams->N = atoi(argv[i+1]); 
			printf("N is %i \n", ioParams->N); 
		}
		else if(strcmp(argv[i],"--io") == 0)
		{
			ioParams->ioLibNum = atoi(argv[i+1]); 
			printf("iolib num is %i \n", ioParams->ioLibNum); 
		}
	} 
	assert(ioParams->N > 0) ;
	assert(ioParams->ioLibNum > -1) ;
	assert(ioParams->ioLibNum < 2) ;
} 

