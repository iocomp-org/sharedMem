#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include <string.h> 
#include <assert.h> 
#include <math.h>
#include "sharedmem.h"

void verify(struct params *ioParams)
{
	MPI_Barrier(ioParams->ioComm); 
	int myRank;
	MPI_Comm_rank(ioParams->ioComm, &myRank); 
#ifndef NDEBUG
		fprintf(ioParams->debug, "Verification started \n"); 
#endif 


#if ADIOS2_USE_MPI
		ioParams->adios_read = adios2_init_config_mpi(CONFIG_FILE_ADIOS2, ioParams->cartcomm);  
#else 
		ioParams->adios_read = adios2_init();  
#endif 
		adios2_set_engine(ioParams->io,ioParams->ADIOS2_IOENGINES[ioParams->ioLibNum-2]); 

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
		fprintf(ioParams->debug, "a[%i] = %lf, b[%i] = %lf, c[%i] = %lf \n", iter, a, iter, b, iter, c); 
#endif 

		// read all the windows and iterations 
		for(int windowNum = 0; windowNum < NUM_WIN; windowNum++)
		{
			// initialise data buffer to store data read locally 
			double* readData; 
			readData = (double *) malloc(sizeof(double)*ioParams->localDataSize); 

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
			
			char FILENAME[100]; 
			strcpy(FILENAME, ioParams->WRITEFILE[windowNum][iter]); 

			switch(ioParams->ioLibNum)
			{
				case(0):
					mpiRead(readData, FILENAME, ioParams ); 
					break; 
				case(1):
					phdf5Read(readData, FILENAME, ioParams); 
					break; 
				case(2): case(3): case(4):
					adios2Read(readData, FILENAME, ioParams); 
					break; 
				default:
					break; 
			} 

			// verify data by checking value by value with STREAM code   
			int test = valueCheck(ioParams, readData, val, windowNum, iter); 
			int test_reduced;  
#ifndef NDEBUG   
					fprintf(ioParams->debug,"Filename %s Data read: \n", ioParams->WRITEFILE[windowNum][iter]); 
					for(int i = 0; i < ioParams->localDataSize; i++)
					{
						fprintf(ioParams->debug,"%lf, ", readData[i]); 
					}
					fprintf(ioParams->debug,"\n"); 
#endif       

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
					printf("Verification passed for filename %s \n", FILENAME); 
				}
			}
			free(readData); 
			readData = NULL; 
		} 
	} 
	
#ifndef NDEBUG   
	fprintf(ioParams->debug,"iodata test freed\n"); 
#endif       
	MPI_Barrier(ioParams->ioComm); 

} 

