#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include "test.h"

void initialise(int* array,int value)
{
	for(int j = 0; j < N; j++)
	{	
		array[j] = value; 
	} 
}

void printData(int* recv)
{
	for(int i = 0; i < N; i++)
	{
		printf("%i ", recv[i]); 
	}
	printf("\n"); 
} 

void computeProcess(int len, MPI_Comm newComm, int* array, int val)
{
		int soi = sizeof(int); 
		int ierr;
		MPI_Win win; 
		ierr = MPI_Win_allocate_shared(soi*len, soi, MPI_INFO_NULL, newComm, &array, &win); 
		error_check(ierr); 

		// create lock and initialise arrays 
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, MPI_MODE_NOCHECK, win);
		initialise(array,val); 
		MPI_Win_unlock(0,win);

		MPI_Barrier(newComm); 
		MPI_Barrier(newComm); 

		ierr = MPI_Win_free(&win);
		error_check(ierr); 
		//printf("MPI window freed by globalrank %i and newRank %i \n",globalRank, newRank); 
} 

void ioProcess(MPI_Comm newComm, int* array)
{
		int ierr; 	
		int soi = sizeof(int); 
		MPI_Win win; 
		ierr = MPI_Win_allocate_shared(0, soi, MPI_INFO_NULL, newComm, &array, &win); 
		error_check(ierr); 

		long int arraySize; 
		int dispUnit; 
		ierr = MPI_Win_shared_query(win, 0, &arraySize, &dispUnit, &array); 
		error_check(ierr); 

		MPI_Barrier(newComm);  // wait for compute process to finish allocating data 
		printData(array); 
		MPI_Barrier(newComm); 

		ierr = MPI_Win_free(&win);
		error_check(ierr); 
		// printf("MPI window freed by globalrank %i and newRank %i \n",globalRank, newRank); 
} 

int main(int argc, char** argv)
{
	//skipped initialization above
	MPI_Init(&argc, &argv);
	MPI_Status status; 
	int ierr; 

	// MPI related initialisations 
	int globalRank, globalSize, colour; 
	MPI_Comm_rank(MPI_COMM_WORLD,&globalRank); 
	MPI_Comm_size(MPI_COMM_WORLD,&globalSize); 
	MPI_Comm newComm; 
	
	int* array1; 
	int* array2; 
	int* array3; 
	int soi = sizeof(int);

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
	printf("Global rank=%i, New rank=%i, Colour=%i \n", globalRank, newRank, colour); 

	// start timer 
	double start, stop, diff; 
	start = MPI_Wtime();

	// comp process initialises array and creates a window with that array 
	if(newRank == 0)  
	{
		computeProcess(N, newComm, array1, 5); 
		computeProcess(N, newComm, array2, 6); 
		computeProcess(N, newComm, array3, 7); 
	} 

	// io process access that memory and prints out the data 
	else 
	{
		ioProcess(newComm, array1); 
		ioProcess(newComm, array2); 
		ioProcess(newComm, array3); 
	} 

	stop = MPI_Wtime();
	diff = stop - start;

	printf("Rank %d, took %8.8fs\n",
			globalRank,  diff);

	MPI_Finalize();
	return 0;
} 

