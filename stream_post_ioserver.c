#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include <string.h> 
#include <assert.h> 
#include "stream_post_ioserver.h"

#define SCALAR 5 
#define STARTING_VAL 1
#define NDIM 1

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


/*
 * function gets shared array pointer using mpi win shared query 
 * and prints out data as proxy for writing to file 
 */
void ioServer(MPI_Comm ioComm, MPI_Comm newComm)
{
	// allocate windows 
	double* array[NUM_WIN]; 
	MPI_Win win_ptr[NUM_WIN]; 
	int flag[NUM_WIN]; 	
	int ierr; 
	int soi = sizeof(double); 

	// timer variables 
	float timers_start[NUM_WIN];
	float timers_end[NUM_WIN];

	// declare array of write files 
	char WRITEFILE[NUM_WIN][10]; 

	// IO setup create cartesian communicators 	
	int ioRank, ioSize,  
			reorder = 0, 
			dims_mpi[NDIM] = { 0 },
			coords[NDIM] = { 0 },
			periods[NDIM] = { 0 };  

	MPI_Comm_rank(ioComm, &ioRank); 
	MPI_Comm_size(ioComm, &ioSize); 

	// Cartesian communicator setup 
	MPI_Comm cartcomm; 
	ierr = MPI_Dims_create(ioSize, NDIM, dims_mpi);
	error_check(ierr);
	ierr = MPI_Cart_create(ioComm, NDIM, dims_mpi, periods, reorder, &cartcomm); //comm
	error_check(ierr);
	ierr = MPI_Cart_coords(cartcomm, ioRank, NDIM, coords);
	error_check(ierr);

	// assign arrray size, subsize and global size 
	int arraysubsize[NDIM], arraygsize[NDIM], arraystart[NDIM]; 
	for(int i = 0; i < NDIM; i++)
	{
		arraysubsize[i] = N; 
		arraygsize[i] = N;
		arraystart[i] = 0; 
	}
	// first element of array start and size different 
	arraystart[0] = N*ioRank;
	arraygsize[0] = N*ioSize; 

	// allocate shared windows 
	for(int i = 0; i < NUM_WIN; i++)
	{
		ierr = MPI_Win_allocate_shared(0, soi, MPI_INFO_NULL, newComm, &array[i], &win_ptr[i]); 
		error_check(ierr);
#ifndef NDEBUG 
		printf("ioServer -> MPI allocated windows %i \n", i); 
#endif 
		// each window can write to its own file, initialise write file name for
		// each window number 
		int arrayNumInt = '0' + i; 
		char arrayNumChar = (char) arrayNumInt; 
		char arrayNumString[] = {arrayNumChar, '\0'}; 
		strcpy(WRITEFILE[i], arrayNumString); 
		char EXT[] = ".dat"; 
		// char EXT[] = "array.h5"; 
		strcat(WRITEFILE[i], EXT); 

		// delete the previous files 
		int test = remove(WRITEFILE[i]);
	} 

	// allocate arrays using window pointers 
	for(int i = 0; i < NUM_WIN; i++)
	{
		long int arraySize; 
		int dispUnit; 
		int ierr; 
		ierr = MPI_Win_shared_query(win_ptr[i], 0, &arraySize, &dispUnit, &array[i]); 
		error_check(ierr); 
#ifndef NDEBUG 
		printf("ioServer -> MPI shared query %i \n", i); 
#endif 
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
#ifndef NDEBUG 
		printf("ioServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
#endif 

		// iterate across all windows 
		for(int i = 0; i < NUM_WIN; i++)
		{
			if(wintestflags[i]>0) // 1 means go for printing 
			{
				// 2 means implement win wait, no need to implement win post again
				if(wintestflags[i]==2) 
				{
#ifndef NDEBUG 
					printf("ioServer -> flag negative and win wait implemented\n"); 
#endif 
					// wait for window completion 
					ierr = MPI_Win_wait(win_ptr[i]); 
					error_check(ierr); 
					fileWrite(array[i], arraysubsize, arraygsize, arraystart, NDIM, cartcomm, WRITEFILE[i], ioComm); 
					timers_end[i] = MPI_Wtime(); // finish writing timer  
				}

				//	Post window for access to array 
				//	and start timer for that window  
				ierr = MPI_Win_post(group, 0, win_ptr[i]);
				error_check(ierr); 
#ifndef NDEBUG 
				printf("ioServer -> post MPI post for window %i \n", i); 
#endif 
				timers_start[i] = MPI_Wtime();

				// test for window completion 	
				ierr = MPI_Win_test(win_ptr[i], &flag[i]); 
				error_check(ierr);
#ifndef NDEBUG 
				printf("ioServer -> win test\n"); 
#endif 
				// if window is available to print then print and end timer 
				if(flag[i])
				{
#ifndef NDEBUG 
					printf("ioServer -> flag positive \n"); 
#endif
					fileWrite(array[i], arraysubsize, arraygsize, arraystart, NDIM, cartcomm, WRITEFILE[i], ioComm); 
					timers_end[i] = MPI_Wtime(); // finish writing timer  
				}
			} 
		}

		// check if no more messages left 
		wintestmult = 1;  // reset value 
		for(int j = 0; j < NUM_WIN; j++)
		{
			wintestmult *= wintestflags[j]; 
		} 
#ifndef NDEBUG 
		printf("ioServer -> wintestmult value %i\n", wintestmult); 
#endif 
	}while(!wintestmult);  // test for completion of all windows 

#ifndef NDEBUG 
	printf("ioServer -> loop server exited \n"); 
#endif 

	// free windows and pointer 
	// while freeing, check if there are any opened windows 
	for(int i = 0; i < NUM_WIN; i++)
	{
		// wait for completion of all windows 
		ierr = MPI_Win_wait(win_ptr[i]); 
		error_check(ierr); 
		fileWrite(array[i], arraysubsize, arraygsize, arraystart, NDIM, cartcomm, WRITEFILE[i], ioComm); 
		timers_end[i] = MPI_Wtime(); // finish writing timer  
#ifndef NDEBUG 
		printf("MPI win free IO server reached\n"); 
#endif 
		
		// MPI_Barrier(MPI_COMM_WORLD); 	
		ierr = MPI_Win_free(&win_ptr[i]);
		error_check(ierr); 
		timers_end[i] = MPI_Wtime(); // finish writing timer  
#ifndef NDEBUG 
		printf("ioServer -> timer ended for array %i, time %lf \n", i, timers_end[i] - timers_start[i]); 
#endif 
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
#ifndef NDEBUG 
	printf("Global rank=%i, New rank=%i, Colour=%i \n", globalRank, newRank, colour); 
#endif 

	// start timer 
	double start, stop, diff; 
	start = MPI_Wtime();

	// divide MPI comm world into compute or I/O server based on newRank which is
	// either 0 or 1 
	int colour_io; 
	MPI_Comm computeComm, ioComm; 
	if(newRank == 0)
	{
		colour = 0; 
		MPI_Comm_split(MPI_COMM_WORLD, colour, globalRank, &computeComm );
	}
	else
	{
		colour = 1; 
		MPI_Comm_split(MPI_COMM_WORLD, colour, globalRank, &ioComm );
	}

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
		double* a; 
		double* b; 
		double* c;  
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
#ifndef NDEBUG 
		printf("coompServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
#endif 
		MPI_Win_start(group, 0, win_A); 
		printf("compServer -> MPI window start with global rank %i \n", globalRank); 
		for(int i = 0; i < N; i++)
		{
			// a[i] = STARTING_VAL;  
			a[i] = i + ((globalRank)*N); 
		}
		MPI_Win_complete(win_A);
#ifndef NDEBUG 
		printf("compServer -> After mpi window unlock for A \n"); 
#endif 

		/* Start STREAM loop */ 
		// COPY
		// send message to ioServer to print via broadcast
		wintestflags[0] = 0;  
		wintestflags[1] = 1; 
		wintestflags[2] = 0; 
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
#endif 
		MPI_Win_start(group, 0, win_C); 
		for(int i = 0; i < N; i++)
		{
			c[i] = a[i]; 
		}
		MPI_Win_complete(win_C); 
#ifndef NDEBUG 
		printf("compServer -> After mpi window unlock for C \n"); 
#endif 

		// SCALE
		// send message to ioServer to print via broadcast
		wintestflags[0] = 0;  
		wintestflags[1] = 0; 
		wintestflags[2] = 1; 
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
#endif 
		MPI_Win_start(group, 0, win_B); 
		for(int i = 0; i < N; i++)
		{
			b[i] = SCALAR * c[i]; 
		}
		MPI_Win_complete(win_B); 
#ifndef NDEBUG 
		printf("compServer -> After mpi window unlock for B \n"); 
#endif 

//		// ADD 
//		// send message to ioServer to print via broadcast
//		wintestflags[0] = 0;  
//		wintestflags[1] = 2; 
//		wintestflags[2] = 0; 
//		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
//#ifndef NDEBUG 
//		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
//#endif 
//
//		MPI_Win_start(group, 0, win_C); 
//#ifndef NDEBUG 
//		printf("compServer -> After win start for C \n"); 
//#endif 
//		for(int i = 0; i < N; i++)
//		{
//			c[i] = a[i] + b[i]; 
//		}
//		MPI_Win_complete(win_C); 
//#ifndef NDEBUG 
//		printf("compServer -> After mpi complete for C \n"); 
//#endif 
//
//		// TRIAD 
//		// send message to ioServer to complete A
//		// then update A
//		// send message to ioServer to print via broadcast
//		wintestflags[0] = 2;  
//		wintestflags[1] = 0; 
//		wintestflags[2] = 0; 
//		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
//#ifndef NDEBUG 
//		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
//#endif 
//
//		MPI_Win_start(group, 0, win_A); 
//		for(int i = 0; i < N; i++)
//		{
//			a[i] = b[i] + SCALAR*c[i]; 
//		}
//		MPI_Win_complete(win_A); 
//#ifndef NDEBUG 
//		printf("compServer -> After mpi complete for A \n"); 
//#endif 
//
//		// send message to ioServer to exit the loop 
		wintestflags[0] = -1;  
		wintestflags[1] = -1; 
 		wintestflags[2] = -1; 
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
//#ifndef NDEBUG 
//		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
//#endif 
//
		// MPI_Barrier(MPI_COMM_WORLD); 	
		dataSendComplete(win_A); 
		dataSendComplete(win_C); 
		dataSendComplete(win_B); 
	} 
	else 
	{
		ioServer(ioComm, newComm); 
	} 

	stop = MPI_Wtime();
	diff = stop - start;

	printf("Rank %d, took %8.8fs\n",
			globalRank,  diff);

	MPI_Finalize();
	return 0;
} 

