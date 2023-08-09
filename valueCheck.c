#include <stdio.h>
#include <stdlib.h>
#include "stream_post_ioserver.h"

int valueCheck(struct params *ioParams, double* iodata_test, double val, int windowNum, int iter)
{
		int test = 0; 
		for(int i = 0; i < ioParams->localDataSize; i++)
		{
			if(iodata_test[i] != val)
			{
				printf("verification failed at index %i for filename %s, read data=%lf, correct val=%lf \n", 
					i, ioParams->WRITEFILE[windowNum][iter], iodata_test[i], val); 
				test = 0; 
				break; 
			}
			else
			{
				test = 1; 
			}
		} 
		return(test); 
} 
