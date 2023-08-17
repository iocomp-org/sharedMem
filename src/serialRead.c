#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "sharedmem.h"

void serialRead(struct params *ioParams, int windowNum, int iter, double val)
{
	FILE *fp;
	size_t num; 
	int test = 0; 

	fp = fopen(ioParams->WRITEFILE[windowNum][iter],"r");
	int ioRank, ioSize;  
	MPI_Comm_rank(ioParams->ioComm, &ioRank); 
	MPI_Comm_size(ioParams->ioComm, &ioSize); 
	
	int globalSize;
	globalSize = ioParams->localDataSize * ioSize; 
	if (fp==NULL)
	{
		printf("Error: file %s not opening. Exiting \n", ioParams->WRITEFILE[windowNum][iter]); 
		error_check(1); 
	}
	
	double* iodata_test; 
	iodata_test = (double *) malloc(sizeof(double)*globalSize); 
	num = fread(iodata_test, sizeof(double), globalSize, fp);
	
	if(num!=globalSize)
	{
		printf("read elements %li not equal to global size of file %i \n", num, globalSize); 
	}
	else
	{
		valueCheck(ioParams, iodata_test, val); 
	} 
	
	// free up variables 
	fclose(fp);
	free(iodata_test); 
	iodata_test = NULL; 
}
