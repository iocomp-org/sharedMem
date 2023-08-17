#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include <string.h> 
#include <assert.h> 
#include <math.h>
#include "stream_post_ioserver.h"
#define FILENAME "ioserver_output.csv"

void verify(struct params *ioParams)
{
	int myRank;
	MPI_Comm_rank(ioParams->ioComm, &myRank); 

	// initialise data buffer to store data read locally 
	double* readData; 
	readData = (double *) malloc(sizeof(double)*ioParams->localDataSize); 

	double a , b, c, val; 
	
	// Initialise 
	a = 1.0;
	b = 2.0; 
	c = 0.0; 
	
	for(int iter = 0; iter < AVGLOOPCOUNT; iter++)
	{
		
		b = SCALAR * c; 

		c = a + b; 

		a = b + (SCALAR*c); 
#ifndef NDEBUG
		printf("a[%i] = %lf, b[%i] = %lf, c[%i] = %lf \n", iter, a, iter, b, iter, c); 
#endif 

		// read all the windows and iterations 
		for(int windowNum = 0; windowNum < NUM_WIN; windowNum++)
		{
			switch(windowNum)
			{
				case(0):
						val = a; 
						break; 
				case(1): 
						val = c; 
						break; 
				case(2): 
						val = b; 
						break; 
				default: 
						break; 
			} 
			
			switch(ioParams->ioLibNum)
			{
				case(0):
					mpiRead(readData, ioParams, windowNum, iter); 
					break; 
				case(1):
					phdf5Read(readData, ioParams, windowNum, iter); 
					break; 
			} 

			// verify data by checking value by value with STREAM code   
			int test = valueCheck(ioParams, readData, val, windowNum, iter); 
			int test_reduced;  

			// sync all values of test, if multiplication comes back as 0 it means
			// verification failed by a particular rank 
			MPI_Reduce(&test, &test_reduced, 1, MPI_INT, MPI_PROD, 0, ioParams->ioComm); 
			if(!myRank)
			{
				if(test_reduced == 0)
				{
					printf("Verification failed \n"); 
				} 
				else
				{
					printf("Verification passed for filename %s \n", ioParams->WRITEFILE[windowNum][iter]); 
				}
			}
		} 
	} 

	free(readData); 
	readData = NULL; 
#ifndef NDEBUG   
	printf("iodata test freed\n"); 
#endif       

} 

