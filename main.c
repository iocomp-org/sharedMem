/*
 *LOOP
 *rank 1: finish writing c
 *rank 0; c=a
 *rank 1: start writing c
 *rank 1: finish writing b
 *rank 0: b=scale(c)
 *rank 1; start writing b
 *rank 1: finish writing c
 *rank 0; c = a+b
 *rank 1: start writing c
 *rank 1: finish writing a
 *rank 0: a = b + scale*c
 *rank 1: start writing a
 *END LOOP
 */ 

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include <string.h> 
#include <assert.h> 
#include <math.h> 
#include "stream_post_ioserver.h"

//void printData(int* recv)
//{
//	for(int i = 0; i < N; i++)
//	{
//		printf("%i ", recv[i]); 
//	}
//	printf("\n"); 
//} 

/* 
 * function recieves pointer array, copies this into allocated shared memory
 * array 
 */ 
struct winElements winAlloc(int len, MPI_Comm newComm)
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
	printf("comp server allocate window \n"); 
#endif 

	output.win = win; 
	output.array = array;  
	return(output); 
}

/*
 * function calls mpi barrier to sync up with io process 
 * so that it finishes accessing data and frees up window 
 */ 
void dataSendComplete(MPI_Win win)
{
	// free window and free shared memory pointer 
#ifndef NDEBUG 
	printf("MPI win free comp server reached\n"); 
#endif 
	int ierr = MPI_Win_free(&win);
	error_check(ierr); 
} 


int main(int argc, char** argv)
{
	//skipped initialization above
	MPI_Init(&argc, &argv);
	int ierr; 

	// MPI related initialisations 
	int globalRank, globalSize, colour; 
	MPI_Comm_rank(MPI_COMM_WORLD,&globalRank); 
	MPI_Comm_size(MPI_COMM_WORLD,&globalSize); 
	MPI_Comm newComm;
	
	// command line interactions to set the problem size and the IO library number  
	struct params ioParams; 
	initialise(argc, argv, &ioParams); 
	
	// brief hello world to check parameters 
	if(!globalRank)
	{
		double problemSize = ioParams.N*8/(pow(2,20)); // MiB 
		printf("Shared memory demonstrator with problem size=%lfMiB, number of windows=%i, number of ranks=%i, I/O library=%i\n", 
		problemSize, NUM_WIN, globalSize, ioParams.ioLibNum); 
	} 

	/*
	 * Assuming IO process and Compute Process are mapped to physical and SMT cores
	 * if size = 10 then IO rank would be 5,6,..9
	 * and compute rank would be 0,1,..4
	 * Assign similar colours to corresponding I/O and compute Process
	 * i.e. rank 0 and rank 5 would have same colour and then same MPI
	 * Communicator 
	 */  
	colour = globalRank%(globalSize/2); // IO rank and comp rank have same colour
	ierr = MPI_Comm_split(MPI_COMM_WORLD, colour, globalRank, &newComm); 
	error_check(ierr); 

	int newRank; 
	ierr = MPI_Comm_rank(newComm,&newRank); 
	error_check(ierr); 
#ifndef NDEBUG 
	printf("Global rank=%i, New rank=%i, Colour=%i \n", globalRank, newRank, colour); 
#endif 

	// divide MPI comm world into compute or I/O server based on newRank which is
	// either 0 or 1 
	// assign compute and I/O rank 
	int colour_io; 
	MPI_Comm computeComm, ioComm; 
	int computeRank, ioRank, computeSize, ioSize;

	if(newRank == 0)
	{
		colour = 0; 
		MPI_Comm_split(MPI_COMM_WORLD, colour, globalRank, &computeComm );
		MPI_Comm_rank(computeComm, &computeRank); 
		MPI_Comm_size(computeComm, &computeSize); 
		if(!computeRank)
		{
			printf("Compute ranks have size %i \n", computeSize); 
		}

	}
	else
	{
		colour = 1; 
		MPI_Comm_split(MPI_COMM_WORLD, colour, globalRank, &ioComm );
		MPI_Comm_rank(ioComm, &ioRank); 
		MPI_Comm_size(ioComm, &ioSize); 
		if(!ioRank)
		{
			printf("IO ranks have size %i \n", ioSize); 
		}
	}

	// initialise windows for each array in both Compute and I/O process
	if(newRank == 0)
	{
		compServer(computeComm, newComm, MPI_COMM_WORLD, &ioParams); 
	} 
	else 
	{
		ioServer(ioComm, newComm,&ioParams); 
	} 

	MPI_Finalize();
	return 0;
} 

