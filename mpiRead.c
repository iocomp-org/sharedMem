#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stream_post_ioserver.h"

void mpiRead(char* FILENAME, MPI_Comm ioServerComm )
{
	FILE *fp;
	size_t num; 
	fp = fopen(FILENAME,"r");
	int ioRank, ioSize;  
	MPI_Comm_rank(ioServerComm, &ioRank); 
	MPI_Comm_size(ioServerComm, &ioSize); 
	
	int globalSize;
	globalSize = N * ioSize; 
	if (fp==NULL)
	{
		printf("Error: file %s not opening. Exiting \n", FILENAME); 
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
		printf("read elements %li  are equal to global size %i \n", num, globalSize); 
		for(int i = 0; i < num; i++)
		{
			printf("%lf, ", iodata_test[i]); 
		} 
		printf("\n"); 
	} 

	// print elements 
	fclose(fp);
	printf("file closed \n"); 
}
