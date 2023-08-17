#include "sharedmem.h"
/* 
 * function recieves pointer array, copies this into allocated shared memory
 * array 
 */ 
struct winElements winAlloc(int len, MPI_Comm newComm, struct params *ioParams)
{
	int soi = sizeof(double); 
	int ierr;
	MPI_Win win; 
	int rank;
	MPI_Comm_rank(newComm, &rank); 
	double *array; 
	struct winElements output; 
	// allocate windows for compute and I/O processes 

	ierr = MPI_Win_allocate_shared(soi*len, soi, MPI_INFO_NULL, newComm, &array, &win); 
	error_check(ierr);
#ifndef NDEBUG 
	fprintf(ioParams->debug, "winAlloc->comp server allocate window \n"); 
#endif 

	output.win = win; 
	output.array = array;  
	return(output); 
}

/*
 * function calls mpi barrier to sync up with io process 
 * so that it finishes accessing data and frees up window 
 */ 
void dataSendComplete(MPI_Win win, struct params *ioParams)
{
	// free window and free shared memory pointer 
#ifndef NDEBUG 
	fprintf(ioParams->debug, "MPI win free comp server reached\n"); 
#endif 
	int ierr = MPI_Win_free(&win);
	error_check(ierr); 
} 

