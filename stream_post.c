#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include "test.h"
#define SCALAR 5 
#define STARTING_VAL 1

void printData(int* recv)
{
	for(int i = 0; i < N; i++)
	{
		printf("%i ", recv[i]); 
	}
	printf("\n"); 
} 

/* 
 * function recieves pointer array, copies this into allocated shared memory
 * array 
 */ 
struct winElements winAlloc(int len, MPI_Comm newComm)
{
	int soi = sizeof(int); 
	int ierr;
	MPI_Win win; 
	int rank;
	MPI_Comm_rank(newComm, &rank); 
	int *array; 
	struct winElements output; 
	// allocate windows for compute and I/O processes 

	if(rank == 0)
	{
		ierr = MPI_Win_allocate_shared(soi*len, soi, MPI_INFO_NULL, newComm, &array, &win); 
		error_check(ierr);
	}
	else
	{
		ierr = MPI_Win_allocate_shared(0, soi, MPI_INFO_NULL, newComm, &array, &win); 
		error_check(ierr);
	}
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
	printf("MPI win free comp server reached\n"); 
	int ierr = MPI_Win_free(&win);
	error_check(ierr); 
} 

/*
 * function gets shared array pointer using mpi win shared query 
 * and prints out data as proxy for writing to file 
 */ 
void ioProcess(MPI_Comm newComm, MPI_Win win, int* array)
{
	long int arraySize; 
	int dispUnit; 
	int ierr; 
	ierr = MPI_Win_shared_query(win, 0, &arraySize, &dispUnit, &array); 
	error_check(ierr); 
	int i; 
	
	// groups 
	MPI_Group comm_group, group;
	int ranks[2]; 
	for (i=0;i<2;i++) {
		ranks[i] = i;     //For forming groups, later
	}
	MPI_Comm_group(newComm,&comm_group);
	
	// form groups 
	/* Compute group consists of rank 0*/
	MPI_Group_incl(comm_group,1,ranks,&group); 
	
	// MPI_Barrier(newComm);  // wait for compute process to finish allocating data 

	printf("Reached MPI post \n"); 
	// Post window for access to array 
	MPI_Win_post(group, 0, win);
	MPI_Win_wait(win); // blocking barrier, ends access 
	printf("MPI wait completed \n"); 

	printData(array); // replace for writing to file  

	printf("MPI window unlocked after printing by ioServer \n"); 
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
	// rank 0: init(a)

	// LOOP

	// rank 1: finish writing c
	// rank 0; c=a
	// rank 1: start writing c
	// rank 1: finish writing b
	// rank 0: b=scale(c)
	// rank 1; start writing b
	// rank 1: finish writing c
	// rank 0; c = a+b
	// rank 1: start writing c
	// rank 1: finish writing a
	// rank 0: a = b + scale*c
	// rank 1: start writing a

	// END LOOP

	// initialise windows for each array in both Compute and I/O process
	MPI_Win win_A, win_B, win_C;
	int* a; 
	int* b; 
	int* c;  
	struct winElements outputWin; 
	// c array 
	outputWin = winAlloc(N, newComm); 
	win_C = outputWin.win; 
	c = outputWin.array; 
	// b array 
	outputWin = winAlloc(N, newComm); 
	win_B = outputWin.win; 
	b = outputWin.array; 
	// a array 
	outputWin = winAlloc(N, newComm); 
	win_A = outputWin.win; 
	a = outputWin.array; 

	// MPI group stuff
	// groups newComm communicator's rank 0 and 1 into a group  

	if(newRank == 0)  
	{
		// comp process initialises array and creates a window with that array 

		MPI_Group comm_group, group;
		int ranks[2]; 
		for (int i=0;i<2;i++) {
			ranks[i] = i;     //For forming groups, later
		}
		MPI_Comm_group(newComm,&comm_group);

		/* I/O group consists of ranks 1*/
		MPI_Group_incl(comm_group,1,ranks+1,&group); 
	
		// INITIALISE A
		MPI_Win_start(group, 0, win_A); 
		for(int i = 0; i < N; i++)
		{
			a[i] = STARTING_VAL;  
		}
		MPI_Win_complete(win_A); 

		// COPY
		MPI_Win_start(group, 0, win_C); 
		printf("After mpi window lock for C \n"); 
		for(int i = 0; i < N; i++)
		{
			c[i] = a[i]; 
		}
		MPI_Win_complete(win_C); 
		printf("After mpi window unlock for C \n"); 
		// MPI_Barrier(newComm); // start printing  

		// SCALE
		MPI_Win_start(group, 0, win_B); 
		printf("After mpi window lock for B \n"); 
		for(int i = 0; i < N; i++)
		{
			b[i] = SCALAR * c[i]; 
		}
		MPI_Win_complete(win_B); 
		printf("After mpi window unlock for C \n"); 
		// MPI_Barrier(newComm); // start printing  

		// ADD 
		MPI_Win_start(group, 0, win_C); 
		printf("After mpi window lock for C \n"); 
		for(int i = 0; i < N; i++)
		{
			c[i] = a[i] + b[i]; 
		}
		MPI_Win_complete(win_C); 
		// MPI_Barrier(newComm); // start printing  

		// TRIAD 
		MPI_Win_start(group, 0, win_A); 
		printf("After mpi window lock for C \n"); 
		for(int i = 0; i < N; i++)
		{
			a[i] = b[i] + SCALAR*c[i]; 
		}
		MPI_Win_complete(win_A); 
		// MPI_Barrier(newComm); // start printing  

	} 
	else 
	{
		// io process access that memory and prints out the data 
		ioProcess(newComm, win_A, a); 
		ioProcess(newComm, win_C, c); 
		ioProcess(newComm, win_B, b); 
		ioProcess(newComm, win_C, c); 
		ioProcess(newComm, win_A, a); 
	} 

	// deallocate windows 
	dataSendComplete(win_B); 
	dataSendComplete(win_C); 
	dataSendComplete(win_A); 

	stop = MPI_Wtime();
	diff = stop - start;

	printf("Rank %d, took %8.8fs\n",
			globalRank,  diff);

	MPI_Finalize();
	return 0;
} 

