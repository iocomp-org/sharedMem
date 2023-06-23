#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include "stream_post_ioserver.h"
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

	ierr = MPI_Win_allocate_shared(soi*len, soi, MPI_INFO_NULL, newComm, &array, &win); 
	error_check(ierr);
	printf("comp server allocate window \n"); 

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
void ioServer(MPI_Comm newComm)
{
	// allocate windows 
	int* array[NUM_WIN]; 
	MPI_Win win_ptr[NUM_WIN]; 
	int flag[NUM_WIN]; 	
	int ierr; 
	int soi = sizeof(int); 
	for(int i = 0; i < NUM_WIN; i++)
	{
		ierr = MPI_Win_allocate_shared(0, soi, MPI_INFO_NULL, newComm, &array[i], &win_ptr[i]); 
		error_check(ierr);
		printf("ioServer -> MPI allocated windows %i \n", i); 
	} 

	for(int i = 0; i < NUM_WIN; i++)
	{
		long int arraySize; 
		int dispUnit; 
		int ierr; 
		ierr = MPI_Win_shared_query(win_ptr[i], 0, &arraySize, &dispUnit, &array[i]); 
		error_check(ierr); 
		printf("ioServer -> MPI shared query %i \n", i); 
	} 
	
	// groups 
	MPI_Group comm_group, group;
	int ranks[2]; 
	for (int j=0; j<2; j++) {
		ranks[j] = j;   
	}
	MPI_Comm_group(newComm,&comm_group);

	/* Compute group consists of rank 0*/
	MPI_Group_incl(comm_group,1,ranks,&group); 
	
	// assign wintestflags int to test for messages from the compute server  
	int wintestflags[NUM_WIN]; 
	// declare mult variable to test for completition among all windows 
	int wintestmult = 1; 
	// Test for window completion 
	do 
	{
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
		printf("ioServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 

		// iterate across all windows 
		for(int i = 0; i < NUM_WIN; i++)
		{
			if(wintestflags[i]==1)
			{
				printf("ioServer -> Reached MPI post for window iteration %i flag multiple %i\n", i, wintestmult); 
				//	Post window for access to array 
				MPI_Win_post(group, 0, win_ptr[i]);

				// wait for window completion 
				//ierr = MPI_Win_wait(win_ptr[i]); 
				//printData(array[i]); // replace for writing to file  

				// test for window completion 	
				for(;;)
				{
					ierr = MPI_Win_test(win_ptr[i], &flag[i]); 
					printf("ioServer -> win test\n"); 
					error_check(ierr);
					if(flag[i])
					{
						printf("ioServer -> flag positive \n"); 
						printData(array[i]); // replace for writing to file  
						break; 
					}
				} 
			} 
		}
		wintestmult = 1;  // reset value 
		for(int j = 0; j < NUM_WIN; j++)
		{
			wintestmult *= wintestflags[j]; 
		} 
	}while(!wintestmult);  // test for completion of all windows 
	
	printf("ioServer -> loop server exited \n"); 

	// free windows and pointer 
	for(int i = 0; i < NUM_WIN; i++)
	{
		printf("MPI win free IO server reached\n"); 
		ierr = MPI_Win_free(&win_ptr[i]);
		error_check(ierr); 
	} 
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
	if(newRank == 0)
	{
		
		// initialise window test flags with 0 
		// assign them 1 if ready for writing 
		int wintestflags[NUM_WIN]; 	
		for(int j = 0; j < NUM_WIN; j++)
		{
			wintestflags[j] = 0; 
		} 
	
		// Allocate windows in order 
		MPI_Win win_A, win_B, win_C;
		int* a; 
		int* b; 
		int* c;  
		struct winElements outputWin; 
		// a array 
		outputWin = winAlloc(N, newComm); 
		win_A = outputWin.win; 
		a = outputWin.array; 

		// c array 
		outputWin = winAlloc(N, newComm); 
		win_C = outputWin.win; 
		c = outputWin.array; 

		// b array 
		outputWin = winAlloc(N, newComm); 
		win_B = outputWin.win; 
		b = outputWin.array; 

		// groups newComm communicator's rank 0 and 1 into a group  
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
		// send message to ioServer to print via broadcast
		wintestflags[0] = 1;  
		wintestflags[1] = 0; 
		wintestflags[2] = 0; 
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
		printf("coompServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
		MPI_Win_start(group, 0, win_A); 
		printf("compServer -> MPI window start \n"); 
		for(int i = 0; i < N; i++)
		{
			a[i] = STARTING_VAL;  
		}
		MPI_Win_complete(win_A);
		printf("compServer -> After mpi window unlock for A \n"); 
		
		/* Start STREAM loop */ 
		// COPY
		// send message to ioServer to print via broadcast
		wintestflags[0] = 0;  
		wintestflags[1] = 1; 
		wintestflags[2] = 0; 
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
		printf("coompServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
		MPI_Win_start(group, 0, win_C); 
		for(int i = 0; i < N; i++)
		{
			c[i] = a[i]; 
		}
		MPI_Win_complete(win_C); 
		printf("compServer -> After mpi window unlock for C \n"); 

		// SCALE
		// send message to ioServer to print via broadcast
		wintestflags[0] = 0;  
		wintestflags[1] = 0; 
		wintestflags[2] = 1; 
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
		MPI_Win_start(group, 0, win_B); 
		for(int i = 0; i < N; i++)
		{
			b[i] = SCALAR * c[i]; 
		}
		MPI_Win_complete(win_B); 
		printf("compServer -> After mpi window unlock for B \n"); 

		// ADD 
		// send message to ioServer to print via broadcast
		wintestflags[0] = 0;  
		wintestflags[1] = 1; 
		wintestflags[2] = 0; 
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
		MPI_Win_start(group, 0, win_C); 
		for(int i = 0; i < N; i++)
		{
			c[i] = a[i] + b[i]; 
		}
		MPI_Win_complete(win_C); 
		printf("compServer -> After mpi complete for C \n"); 
		
		
		// TRIAD 
		// send message to ioServer to complete A
		// then update A
		// send message to ioServer to print via broadcast
		wintestflags[0] = 1;  
		wintestflags[1] = 0; 
		wintestflags[2] = 0; 
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 

		MPI_Win_start(group, 0, win_A); 
		for(int i = 0; i < N; i++)
		{
			a[i] = b[i] + SCALAR*c[i]; 
		}
		MPI_Win_complete(win_A); 
		printf("compServer -> After mpi complete for A \n"); 

		// send message to ioServer to exit the loop 
		wintestflags[0] = -1;  
		wintestflags[1] = -1; 
		wintestflags[2] = -1; 
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 

		dataSendComplete(win_A); 
		dataSendComplete(win_C); 
		dataSendComplete(win_B); 

	} 
	else 
	{
		ioServer(newComm); 
	} 



	stop = MPI_Wtime();
	diff = stop - start;

	printf("Rank %d, took %8.8fs\n",
			globalRank,  diff);

	MPI_Finalize();
	return 0;
} 

