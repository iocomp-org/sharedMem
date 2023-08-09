#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <memory.h>
#include "stream_post_ioserver.h"

void mpiRead(struct params *ioParams, int windowNum, int iter, double val)
{
	int i, ierr, nprocs, myrank; 

	int dims[NDIM],
			coords[NDIM], 
			periods[NDIM]; 

	int order = 0, 
			disp = 0; // number of bytes to be skipped from the start. ex headers. 

	// initialise data buffer to store data read locally 
	double* iodata_test; 
	iodata_test = (double *) malloc(sizeof(double)*ioParams->localDataSize); 

	// convert size_t array parameters to int arrays using MPI and HDF5 
	int localArray[NDIM]; 
	int globalArray[NDIM]; 
	int arrayStart[NDIM]; 
	for(int i = 0; i < NDIM; i++)
	{
		localArray[i] = (int)ioParams->localArray[i]; 
		globalArray[i] = (int)ioParams->globalArray[i]; 
		arrayStart[i] = (int)ioParams->arrayStart[i]; 
	}

	// MPI initialisations
	MPI_File        fh; 
	MPI_Status      status;
	MPI_Datatype    filetype, mpi_subarray; 

	MPI_Comm_size(ioParams->cartcomm, &nprocs);
	MPI_Comm_rank(ioParams->cartcomm, &myrank);

	ierr = MPI_Cart_get(ioParams->cartcomm, NDIM, dims, periods, coords); 
	error_check(ierr); 

	ierr = MPI_File_open(ioParams->cartcomm, ioParams->WRITEFILE[windowNum][iter],
			MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh); 
	error_check(ierr); 
	MPI_Info info  = MPI_INFO_NULL; 

#ifndef NDEBUG   
	printf("Cartcomm rank and size %i %i \n", myrank, nprocs); 
	printf("arraygsize for rank %i : %i  \n",myrank, globalArray[0]	); 
	printf("arraysubsize for rank %i : %i \n",myrank,localArray[0]); 
	printf("arraystart for rank %i : %i  \n",myrank,arrayStart[0]		); 
#endif 

	ierr = MPI_Type_create_subarray(NDIM, globalArray, localArray, arrayStart,
			MPI_ORDER_C, MPI_DOUBLE, &filetype); 
	error_check(ierr); 
#ifndef NDEBUG   
	printf("MPI create subarray \n"); 
#endif                                   

	ierr = MPI_Type_commit(&filetype); 
	error_check(ierr); 
#ifndef NDEBUG   
	printf("MPI type commit \n"); 
#endif       

	ierr = MPI_File_open(ioParams->cartcomm, ioParams->WRITEFILE[windowNum][iter], MPI_MODE_RDONLY, 
			MPI_INFO_NULL, &fh); 
	error_check(ierr); 
#ifndef NDEBUG   
	printf("MPI file open \n"); 
#endif       

	// Set view for this process using datatype 
	ierr = MPI_File_set_view(fh, 0, MPI_DOUBLE, filetype, "native",  
			MPI_INFO_NULL); 
	error_check(ierr); 
#ifndef NDEBUG   
	printf("MPI file set view \n"); 
#endif       

	// Remove halo data    
	int total_data = 1; 
	for (i = 0; i< NDIM; i++)
	{
		total_data *= localArray[i]; 
	}
	ierr = MPI_File_read_all(fh, iodata_test, total_data, MPI_DOUBLE, &status);   
	error_check(ierr); 
#ifndef NDEBUG   
	printf("MPI file write all \n"); 
#endif       

	// verify data 
	int test = valueCheck(ioParams, iodata_test, val, windowNum, iter); 
	int test_reduced;  

	// sync all values of test, if multiplication comes back as 0 it means
	// verification failed 
	MPI_Reduce(&test, &test_reduced, 1, MPI_INT, MPI_PROD, 0, ioParams->ioComm); 
	if(!myrank)
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
	

	ierr = MPI_File_close(&fh);
	error_check(ierr); 
#ifndef NDEBUG   
	printf("MPI file close \n"); 
#endif       

	ierr = MPI_Type_free(&filetype); 
	error_check(ierr); 
#ifndef NDEBUG   
	printf("MPI filetype\n"); 
#endif       
}


