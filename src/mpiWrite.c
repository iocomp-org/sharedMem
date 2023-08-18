#include <omp.h> 
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "sharedmem.h"

void mpiiowrite(double* iodata, char* FILENAME, struct params* ioParams )
{   
	int i, ierr, nprocs, myrank; 

	int dims[NDIM],
			coords[NDIM], 
			periods[NDIM]; 

	// MPI initialisations
	MPI_File        fh; 
	MPI_Status      status;
	MPI_Datatype    filetype; 

	// convert size_t array parameters to int arrays using MPI 
	int localArray[NDIM]; 
	int globalArray[NDIM]; 
	int arrayStart[NDIM]; 
	for(int i = 0; i < NDIM; i++)
	{
		localArray[i] = (int)ioParams->localArray[i]; 
		globalArray[i] = (int)ioParams->globalArray[i]; 
		arrayStart[i] = (int)ioParams->arrayStart[i]; 
	}

	MPI_Comm_size(ioParams->cartcomm, &nprocs);
	MPI_Comm_rank(ioParams->cartcomm, &myrank);

	ierr = MPI_Cart_get(ioParams->cartcomm, NDIM, dims, periods, coords); 
	error_check(ierr); 

	ierr = MPI_File_open(ioParams->cartcomm, FILENAME,
			MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh); 
	error_check(ierr); 

#ifndef NDEBUG   
	fprintf(ioParams->debug,"Cartcomm rank and size %i %i \n", myrank, nprocs); 
	fprintf(ioParams->debug,"arraygsize for rank %i : %i  \n",myrank, globalArray[0]	); 
	fprintf(ioParams->debug,"arraysubsize for rank %i : %i \n",myrank,localArray[0]); 
	fprintf(ioParams->debug,"arraystart for rank %i : %i  \n",myrank,arrayStart[0]		); 
#endif 

	ierr = MPI_Type_create_subarray(NDIM, globalArray, localArray, arrayStart,
			MPI_ORDER_C, MPI_DOUBLE, &filetype); 
	error_check(ierr); 
#ifndef NDEBUG   
	fprintf(ioParams->debug,"MPI create subarray \n"); 
#endif                                   

	ierr = MPI_Type_commit(&filetype); 
	error_check(ierr); 
#ifndef NDEBUG   
	fprintf(ioParams->debug,"MPI type commit \n"); 
#endif       

	ierr = MPI_File_open(ioParams->cartcomm, FILENAME, MPI_MODE_CREATE | MPI_MODE_WRONLY, 
			MPI_INFO_NULL, &fh); 
	error_check(ierr); 
#ifndef NDEBUG   
	fprintf(ioParams->debug,"MPI file open \n"); 
#endif       

	// Set view for this process using datatype 
	ierr = MPI_File_set_view(fh, 0, MPI_DOUBLE, filetype, "native",  
			MPI_INFO_NULL); 
	error_check(ierr); 
#ifndef NDEBUG   
	fprintf(ioParams->debug,"MPI file set view \n"); 
#endif       

	// Remove halo data    
	int total_data = 1; 
	for (i = 0; i< NDIM; i++)
	{
		total_data *= localArray[i]; 
	}
	ierr = MPI_File_write_all(fh, iodata, total_data, MPI_DOUBLE, &status);   
	error_check(ierr); 
#ifndef NDEBUG   
	fprintf(ioParams->debug,"MPI file write all \n"); 
#endif       

	ierr = MPI_File_close(&fh);
	error_check(ierr); 
#ifndef NDEBUG   
	fprintf(ioParams->debug,"MPI file close \n"); 
#endif       

	ierr = MPI_Type_free(&filetype); 
	error_check(ierr); 
#ifndef NDEBUG   
	fprintf(ioParams->debug,"MPI filetype\n"); 
#endif       
}


