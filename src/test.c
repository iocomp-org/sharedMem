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

MPI_Win createWindow(int len, MPI_Comm newComm, int* array)
{
	int soi = sizeof(int); 
	int ierr;
	MPI_Win win; 
	ierr = MPI_Win_allocate_shared(soi*len, soi, MPI_INFO_NULL, newComm, &array, &win); 
	error_check(ierr); 
	return win; 
} 

void work_block(int* array) // resembles COPY function of STREAM 
{
	int i = 0; 
	int* COPY;
	COPY = (int *)(malloc(sizeof(int) * N)); 
	if(COPY == NULL) 
	{
		printf("malloc issue, quitting \n"); 	
		exit(1); 
	} 
	for(i = 0; i < N; i ++)
	{
		COPY[i] = array[i]; 		
	}
	free(COPY); 
	COPY = NULL; 
}

void computeProcess(int len, MPI_Comm newComm, int val)
{
	int soi = sizeof(int); 
	int* array; 
	int* array2; 
	int ierr;
	MPI_Win win; 
	MPI_Win win2; 
	ierr = MPI_Win_allocate_shared(soi*len, soi, MPI_INFO_NULL, newComm, &array, &win); 
	error_check(ierr); 
	ierr = MPI_Win_allocate_shared(soi*len, soi, MPI_INFO_NULL, newComm, &array2, &win2); 
	error_check(ierr); 
	//MPI_Win win = createWindow(len, newComm, array); 

	// create lock and initialise arrays 
	MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, MPI_MODE_NOCHECK, win);
	printf("After mpi window lock \n"); 
	initialise(array,val); 
	printf("After initialising array \n"); 
	MPI_Win_unlock(0,win);
	printf("After mpi window unlock \n"); 

	printf("reached MPI barrier compute process \n"); 
	MPI_Barrier(newComm); 
	work_block(array); // do work while I/O process is writing to file 
	printf("reached 2nd MPI barrier compute process \n"); 
	// MPI_Barrier(newComm); 

	ierr = MPI_Win_free(&win);
	error_check(ierr); 

	// 2 window creation 
	MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, MPI_MODE_NOCHECK, win2);
	printf("After mpi window lock \n"); 
	initialise(array2,val); 
	printf("After initialising array \n"); 
	MPI_Win_unlock(0,win2);
	printf("After mpi window unlock \n"); 

	MPI_Barrier(newComm); 
	work_block(array2); // do work while I/O process is writing to file 
	// MPI_Barrier(newComm); 

	ierr = MPI_Win_free(&win2);
	error_check(ierr); 

} 

void ioProcess(MPI_Comm newComm)
{
	int* array; 	
	int* array2; 	
	int ierr; 	
	int soi = sizeof(int); 
	MPI_Win win; 
	MPI_Win win2; 
	ierr = MPI_Win_allocate_shared(0, soi, MPI_INFO_NULL, newComm, &array, &win); 
	error_check(ierr); 

	ierr = MPI_Win_allocate_shared(0, soi, MPI_INFO_NULL, newComm, &array2, &win2); 
	error_check(ierr); 

	long int arraySize; 
	int dispUnit; 
	ierr = MPI_Win_shared_query(win, 0, &arraySize, &dispUnit, &array); 
	error_check(ierr); 

	long int arraySize2; 
	int dispUnit2; 
	ierr = MPI_Win_shared_query(win2, 0, &arraySize2, &dispUnit2, &array2); 
	error_check(ierr); 
	
	printf("reached MPI barrier io process \n"); 
	MPI_Barrier(newComm);  // wait for compute process to finish allocating data 
	printData(array); // replace for writing to file  
	ierr = MPI_Win_free(&win);
	error_check(ierr); 

	printf("MPI window free IO process\n"); 

	//printf("reached 2 MPI barrier IO process\n"); 
	//MPI_Barrier(newComm);  // wait for compute process to finish allocating data 

	MPI_Barrier(newComm); 
	printData(array2); // replace for writing to file  
	ierr = MPI_Win_free(&win2);
	error_check(ierr); 
	// MPI_Barrier(newComm);  // wait for compute process to finish allocating data 
	// printf("MPI window freed by globalrank %i and newRank %i \n",globalRank, newRank); 
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
		computeProcess(N, newComm,  5); 
		//computeProcess(N, newComm,  6); 
		//computeProcess(N, newComm,  7); 
	} 

	// io process access that memory and prints out the data 
	else 
	{
		ioProcess(newComm); 
		// ioProcess(newComm); 
		//ioProcess(newComm); 
	} 

	stop = MPI_Wtime();
	diff = stop - start;

	printf("Rank %d, took %8.8fs\n",
			globalRank,  diff);

	MPI_Finalize();
	return 0;
} 

