#include <omp.h> 
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "stream_post_ioserver.h"

void mpiiowrite(double* iodata, int*arraysubsize, int* arraygsize, int* arraystart, int NDIM, MPI_Comm cartcomm, char* FILENAME)
{   
	int i, ierr, nprocs, myrank; 

	int dims[NDIM],
			coords[NDIM], 
			periods[NDIM]; 

	int order = 0, 
			disp = 0; // number of bytes to be skipped from the start. ex headers. 

	// MPI initialisations
	MPI_File        fh; 
	MPI_Status      status;
	MPI_Datatype    filetype, mpi_subarray; 

	MPI_Comm_size(cartcomm, &nprocs);
	MPI_Comm_rank(cartcomm, &myrank);

	printf("Cartcomm rank and size %i %i \n", myrank, nprocs); 
	ierr = MPI_Cart_get(cartcomm, NDIM, dims, periods, coords); 
	error_check(ierr); 

	ierr = MPI_File_open(cartcomm, FILENAME,
			MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh); 
	error_check(ierr); 
	MPI_Info info  = MPI_INFO_NULL; 

#ifndef NDEBUG   
	printf("MPI cartcomm got ranks \n"); 
	printf("arraygsize for rank %i : %i  \n",myrank, arraygsize[0]	); 
	printf("arraysubsize for rank %i : %i \n",myrank,arraysubsize[0]); 
	printf("arraystart for rank %i : %i  \n",myrank,arraystart[0]		); 
#endif 

	ierr = MPI_Type_create_subarray(NDIM, arraygsize, arraysubsize, arraystart,
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

	ierr = MPI_File_open(cartcomm, FILENAME, MPI_MODE_CREATE | MPI_MODE_WRONLY, 
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
		total_data *= arraysubsize[i]; 
	}
	ierr = MPI_File_write_all(fh, iodata, total_data, MPI_DOUBLE, &status);   
	error_check(ierr); 
#ifndef NDEBUG   
	printf("MPI file write all \n"); 
#endif       

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


