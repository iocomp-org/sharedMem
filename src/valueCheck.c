#include <stdio.h>
#include <stdlib.h>
#include "sharedmem.h"

int valueCheck(struct params *ioParams, double* iodata_test, double val, int windowNum, int iter)
{
		int test = 1; 
		int ioRank;
		MPI_Comm_rank(ioParams->ioComm, &ioRank); 
		for(int i = 0; i < ioParams->localDataSize; i++)
		{
			if(iodata_test[i] != val)
			{
				printf("Verification failed for I/O rank %i at index %i for filename %s, read data=%lf, correct val=%lf \n", 
					ioRank, i, ioParams->WRITEFILE[windowNum][iter], iodata_test[i], val); 
				test = 0; 
				break; 
			}
		} 
		return(test); 
} 
