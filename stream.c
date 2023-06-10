#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include "test.h"
#define SCALAR 5 
#define STARTING_VAL 1

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

// function recieves pointer array, copies this into allocated shared memory
// array 
MPI_Win dataSend(int len, MPI_Comm newComm, int* values)
{
	int soi = sizeof(int); 
	int* array; 
	int ierr;
	MPI_Win win; 
	ierr = MPI_Win_allocate_shared(soi*len, soi, MPI_INFO_NULL, newComm, &array, &win); 
	error_check(ierr); 
	//MPI_Win win = createWindow(len, newComm, array); 

	// create lock and initialise arrays 
	MPI_Win_lock(MPI_LOCK_SHARED, 0, MPI_MODE_NOCHECK, win);
	printf("After mpi window lock \n"); 
	
	// copy array from values into shared array 
	for(int i = 0; i < N; i ++)
	{
		array[i] = values[i]; 
	} 
	printf("After initialising array \n"); 
	MPI_Win_unlock(0,win);
	printf("After mpi window unlock \n"); 

	MPI_Barrier(newComm);  // signal to I/O process to start working
	
	return(win); 
	
} 

void dataSendComplete(MPI_Comm newComm, MPI_Win win)
{
	// signal to I/O process to finish working
	MPI_Barrier(newComm); 
	
	// free window and free shared memory pointer 
	int ierr = MPI_Win_free(&win);
	error_check(ierr); 
} 

void ioProcess(MPI_Comm newComm)
{
	int* array; 	
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
	printData(array); // replace for writing to file  
	MPI_Barrier(newComm); 

	ierr = MPI_Win_free(&win);
	error_check(ierr); 
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
		MPI_Win win_copy, win_add, win_scale, win_triad; 
		int* a; 
		a = (int *)malloc(sizeof(int)*N); 
		if(a == NULL)
		{
			exit(1); 
		}
		int* c; 
		c = (int *)malloc(sizeof(int)*N); 
		if(c == NULL)
		{
			exit(1); 
		}
		int* b; 
		b = (int *)malloc(sizeof(int)*N); 
		if(b == NULL)
		{
			exit(1); 
		}
		
		// initialise a
		for(int i = 0; i < N; i++)
		{
			a[i] = STARTING_VAL;  
		}

		// COPY
		for(int i = 0; i < N; i++)
		{
			c[i] = a[i]; 
		}
		win_copy = dataSend(N, newComm,  c); 
		// wait for C 
		dataSendComplete(newComm, win_copy); 

		// SCALE
		for(int i = 0; i < N; i++)
		{
			b[i] = SCALAR * c[i]; 
		}
		win_scale = dataSend(N, newComm,  b); 
		// wait for B
		dataSendComplete(newComm, win_scale); 

		// ADD 
		for(int i = 0; i < N; i++)
		{
			c[i] = a[i] + b[i]; 
		}
		win_add = dataSend(N, newComm,  c); 
		// wait for C	
		dataSendComplete(newComm, win_add); 

		// TRIAD 
		for(int i = 0; i < N; i++)
		{
			a[i] = b[i] + SCALAR*c[i]; 
		}
		win_triad = dataSend(N, newComm,  a); 
		// wait for A
		dataSendComplete(newComm, win_triad); 
	} 

	// io process access that memory and prints out the data 
	else 
	{
		ioProcess(newComm); 
		ioProcess(newComm); 
		ioProcess(newComm); 
		ioProcess(newComm); 
	} 

	stop = MPI_Wtime();
	diff = stop - start;

	printf("Rank %d, took %8.8fs\n",
			globalRank,  diff);

	MPI_Finalize();
	return 0;
} 

