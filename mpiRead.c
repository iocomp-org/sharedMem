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
	
	int* iodata_test; 
	iodata_test = (int *) malloc(sizeof(int)*globalSize); 
	num = fread(iodata_test, sizeof(int), globalSize, fp);
	
	if(num!=globalSize)
	{
		printf("read elements %li not equal to global size of file %i \n", num, N); 
	}
	else
	{
		for(int i = 0; i < num; i++)
		{
			printf("%i, ", iodata_test[i]); 
		} 
		printf("\n"); 
	} 

	// print elements 
	fclose(fp);
	printf("file closed \n"); 
}
