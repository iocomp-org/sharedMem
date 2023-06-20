#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include "test.h"
#define SCALAR 5 
#define STARTING_VAL 1
#define MAX_LOOP 5

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
	int rank;
	MPI_Comm_rank(newComm, &rank); 
	struct winElements output; 
	// allocate windows for compute and I/O processes 

	if(rank == 0)
	{
		ierr = MPI_Win_allocate_shared(soi*len, soi, MPI_INFO_NULL, newComm, &(output.array), &(output.win) ); 
		error_check(ierr);
		printf("Compute rank allocate window \n"); 
	}
	//	else
	//	{
	//		ierr = MPI_Win_allocate_shared(0, soi, MPI_INFO_NULL, newComm, &(output.array), &(output.win) ); 
	//		error_check(ierr);
	//		printf("I/O rank allocate window \n"); 
	//	}
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
 * io Server function should serve as a constant checker for MPI shared window
 * completion
 */ 
void ioServer(MPI_Win win_ptr, MPI_Comm newComm, MPI_Group group, int num)
{
	int ierr; 
	int flag_mult = 1;  
	int flag;
	int* array; // array of pointers
							// array = (int** )malloc(sizeof(int*)*num); 

	printf("io server reached \n"); 
	// access windows 
	
	int soi = sizeof(int); 
	ierr = MPI_Win_allocate_shared(0, soi, MPI_INFO_NULL, newComm, &array, &win_ptr); 
	error_check(ierr);
	printf("I/O rank allocate window \n"); 
	for(int i = 0 ; i < num; i++)
	{
		long int arraySize; 
		int dispUnit; 
		int ierr; 
		ierr = MPI_Win_shared_query(win_ptr, 0, &arraySize, &dispUnit, &array); 
		error_check(ierr); 
		int i; 

		// grant access to window shared array  
		// MPI_Win_post(group, 0, win_ptr[num]);
		ierr = MPI_Win_post(group, 0, win_ptr);
		error_check(ierr);

		for(;;)
		{
			ierr = MPI_Win_test(win_ptr, &flag); 
			error_check(ierr);
			printf("after window testing \n"); 
			if(flag)
			{
				printData(array); // replace for writing to file  
				printf("flag positive \n"); 
				break; 
			}
		} 
		// MPI_Win_wait(win_ptr); // blocking barrier, ends access 
	} 

	// free window and free shared memory pointer 
	printf("MPI win free io server reached\n"); 
	ierr = MPI_Win_free(&win_ptr);
	error_check(ierr); 

	// ierr = MPI_Win_test(win_ptr[0], &flag); 
	//	ierr = MPI_Win_test(win_ptr, &flag); 
	//	error_check(ierr);
	//	printf("after window testing \n"); 
	//	if(flag)
	//	{
	//		printData(array[num]); // replace for writing to file  
	//	}

	//// infinite do while loop 
	//do{
	//	for(int i = 0; i < num; i++)
	//	{
	//		ierr = MPI_Win_test(win_ptr[i], &flag); 
	//		error_check(ierr);
	//		if(flag)
	//		{
	//			printData(array[num]); // replace for writing to file  
	//		}
	//		flag_mult *= flag; 
	//	}
	//}while(!flag_mult); // check if all flag values are positive and only then end the loop. 

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
	// rank 0: initialise a
	// rank 1: print a 
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

	MPI_Group comm_group, group;

	int ranks[2]; 
	for (int i =0;i<2;i++) {
		ranks[i] = i;     //For forming groups, later
	}
	// initialise groups by compute and I/O processes 
	if(newRank == 0) 
	{
		// groups 
		MPI_Comm_group(newComm,&comm_group);

		/* Compute group consists of ranks 1*/
		printf("Groups going to be included comp server \n"); 
		MPI_Group_incl(comm_group,1,ranks+1,&group); 
		printf("Groups included comp server \n"); 
	} 
	else
	{
		// groups 
		MPI_Comm_group(newComm,&comm_group);

		/* IO group consists of rank 0*/
		printf("Groups going to be included I/O server \n"); 
		MPI_Group_incl(comm_group,1,ranks,&group); 
		printf("groups included  IO server \n"); 
	} 

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
	//	// b array 
	//	outputWin = winAlloc(N, newComm); 
	//	win_B = outputWin.win; 
	//	b = outputWin.array; 
	//	// a array 
	//	outputWin = winAlloc(N, newComm); 
	//	win_A = outputWin.win; 
	//	a = outputWin.array; 

	// MPI window pointers
	// MPI_Win* win_ptr; 
	// win_ptr = &win_C; 
	// win_ptr[0] = win_C; 
	//	win_ptr[0] = win_A; 
	//	win_ptr[1] = win_B; 
	//	win_ptr[2] = win_C; 


	//	// INITIALISE A
	//	if(newRank == 0)
	//	{
	//		MPI_Win_start(group, 0, win_A); 
	//		for(int i = 0; i < N; i++)
	//		{
	//			a[i] = STARTING_VAL;  
	//		}
	//		MPI_Win_complete(win_A); 
	//	} 

	//	MPI_Barrier(newComm); 
	//	printf("A initialisation complete \n"); 

	// STREAM compute kernels loop 
	//	for(int loop = 0; loop < MAX_LOOP; loop ++)
	//	{
	if(newRank == 0)  
	{
		// COPY
		printf("Entered loop for compute \n"); 
		MPI_Win_start(group, 0, win_C); 
		printf("After mpi window lock for C \n"); 
		for(int i = 0; i < N; i++)
		{
			// c[i] = a[i]; 
			c[i] = 5; 
		}
		MPI_Win_complete(win_C); 
		printf("After mpi window unlock for C \n"); 

		dataSendComplete(win_C); 

		//			// SCALE
		//			MPI_Win_start(group, 0, win_B); 
		//			printf("After mpi window lock for B \n"); 
		//			for(int i = 0; i < N; i++)
		//			{
		//				b[i] = SCALAR * c[i]; 
		//			}
		//			MPI_Win_complete(win_B); 
		//			printf("After mpi window unlock for C \n"); 
		//			// MPI_Barrier(newComm); // start printing  
		//
		//			// ADD 
		//			MPI_Win_start(group, 0, win_C); 
		//			printf("After mpi window lock for C \n"); 
		//			for(int i = 0; i < N; i++)
		//			{
		//				c[i] = a[i] + b[i]; 
		//			}
		//			MPI_Win_complete(win_C); 
		//			// MPI_Barrier(newComm); // start printing  
		//
		//			// TRIAD 
		//			MPI_Win_start(group, 0, win_A); 
		//			printf("After mpi window lock for C \n"); 
		//			for(int i = 0; i < N; i++)
		//			{
		//				a[i] = b[i] + SCALAR*c[i]; 
		//			}
		//			MPI_Win_complete(win_A); 
		//			// MPI_Barrier(newComm); // start printing  

	} 
	else 
	{
		// io process access that memory and prints out the data 
		//			ioProcess(newComm, win_A, a); 
		//			ioProcess(newComm, win_C, c); 
		//			ioProcess(newComm, win_B, b); 
		//			ioProcess(newComm, win_C, c); 
		//			ioProcess(newComm, win_A, a); 
		ioServer(win_C, newComm, group, 1); 
	} 
	//	} 

	printf("deallocate windows \n"); 
	// deallocate windows 
	//dataSendComplete(win_B); 
	//dataSendComplete(win_A); 
	// dataSendComplete(win_C); 

	stop = MPI_Wtime();
	diff = stop - start;

	printf("Rank %d, took %8.8fs\n",
			globalRank,  diff);

	MPI_Finalize();
	return 0;
} 

