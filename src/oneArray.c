#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#define N 10 
#define error_check(ierr) if(ierr!=MPI_SUCCESS){ printf("mpi error \n"); return(1);  }  

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

	// allocate memory
	int soi = sizeof(int);
	int* array; 
	// array = (int *)malloc(soi*N); 
	// int array[N]; 

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
	// newComm = MPI_COMM_WORLD; 
	// test
	int newRank; 
	ierr = MPI_Comm_rank(newComm,&newRank); 
	error_check(ierr); 
	printf("Global rank=%i, New rank=%i, Colour=%i \n", globalRank, newRank, colour); 
	MPI_Win win; 

	// start timer 
	double start, stop, diff; 
	start = MPI_Wtime();

	// comp process initialises array and creates a window with that array 

	if(newRank == 0)  
	{
		ierr = MPI_Win_allocate_shared(soi*N, soi, MPI_INFO_NULL, newComm, &array, &win); 
		error_check(ierr); 
		printf("memory initialised by globalrank %i and newRank %i \n",globalRank, newRank); 
		//ierr = MPI_Win_create(array, soi*N, soi, MPI_INFO_NULL, newComm, &win);

		// create lock and initialise arrays 
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, MPI_MODE_NOCHECK, win);
		initialise(array,5); 
		MPI_Win_unlock(0,win);
	} 
	else 
	{
		// non 0 rank creates window using NULL pointer 
		// printf("null window created by globalrank %i and newRank %i \n",globalRank, newRank); 
		// ierr = MPI_Win_create(NULL, 0,soi, MPI_INFO_NULL, newComm, &win);
		ierr = MPI_Win_allocate_shared(0, soi, MPI_INFO_NULL, newComm, &array, &win); 
		error_check(ierr); 
		long int arraySize; 
		int dispUnit; 
		ierr = MPI_Win_shared_query(win, 0, &arraySize, &dispUnit, &array); 
		error_check(ierr); 
		printf("MPI shared query globalrank %i and newRank %i \n",globalRank, newRank); 
	} 

	MPI_Barrier(newComm); 

	if(newRank!=0)
	{
		printData(array); 
	} 

	MPI_Barrier(newComm); 

	// start window 
	printf("MPI window fence created globalrank %i and newRank %i \n",globalRank, newRank); 
	//ierr = MPI_Win_fence(MPI_MODE_NOPRECEDE,win);
	//error_check(ierr); 

	//ierr = MPI_Win_fence(MPI_MODE_NOSUCCEED, win);
	//error_check(ierr); 
	ierr = MPI_Win_free(&win);
	error_check(ierr); 
	printf("MPI window freed by globalrank %i and newRank %i \n",globalRank, newRank); 
	// end window

	// print values 
	if(newRank != 0)
	{
		printf("Global rank %i reading array with value set to be 5 \n",globalRank); 
		// printData(array); 
	} 

	stop = MPI_Wtime();
	diff = stop - start;

	printf("Rank %d, took %8.8fs\n",
			globalRank,  diff);
	
	// free(array); 
	// array = NULL; 
	// printf("array freed \n"); 

	MPI_Finalize();
	printf("MPI finalise \n"); 
	return 0;
} 

