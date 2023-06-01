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
	// int* array = (int *)malloc(soi*N); 
	int array[N]; 

	/*
	 * Assuming IO process and Compute Process are mapped to physical and SMT cores
	 * if size = 10 then IO rank would be 5,6,..9
	 * and compute rank would be 0,1,..4
	 * Assign similar colours to corresponding I/O and compute Process
	 * i.e. rank 0 and rank 5 would have same colour and then same MPI
	 * Communicator 
	 */  
	colour = globalRank%(globalSize/2); // IO rank and comp rank have same colour
	MPI_Comm_split(MPI_COMM_WORLD, colour, globalRank, &newComm); 
	// test
	int newRank; 
	MPI_Comm_rank(newComm,&newRank); 
	printf("Global rank=%i, New rank=%i, Colour=%i \n", globalRank, newRank, colour); 
	MPI_Win win; 


//	// timers 
//	double start, stop, diff; 
//
//	start = MPI_Wtime();
//
//	if(newRank == 0) // comp process 
//	{
//		printf("memory initialised by newRank %i \n", newRank); 
//		ierr = MPI_Win_create(array, soi*N, soi, MPI_INFO_NULL, newComm, &win);
//		error_check(ierr); 
//	} 
//
//	else
//	{
//		// non 0 rank creates window using NULL pointer 
//		ierr = MPI_Win_create(NULL, 0,1, MPI_INFO_NULL, newComm, &win);
//		error_check(ierr); 
//	} 
//	
//	if(newRank == 0)
//	{
//		initialise(array, 5); 
//	}
//	else
//	{
//		initialise(array, 7); 
//	}
//	// start window 
//	MPI_Win_fence(MPI_MODE_NOPRECEDE,win);
//	if(newRank != 0)
//	{
//		MPI_Get(array, N, MPI_INT, 0,0,N, MPI_INT, win); 
//	} 
//	MPI_Win_fence(MPI_MODE_NOSUCCEED, win);
//
//	MPI_Win_free(&win);
//	// end window
//	
//	// print values 
//	if(newRank != 0)
//	{
//		printf("Global rank %i reading array with value set to be 5 \n",globalRank); 
//		printData(array); 
//	} 
//
//	stop = MPI_Wtime();
//	diff = stop - start;
//
//	printf("Rank %d, took %8.8fs\n",
//			globalRank,  diff);

	MPI_Finalize();
	return 0;
} 

