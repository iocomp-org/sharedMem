#include <stdio.h>
#include <mpi.h>
#define N 10 

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

	int globalRank, globalSize, colour; 
	MPI_Comm_rank(MPI_COMM_WORLD,&globalRank); 
	MPI_Comm_size(MPI_COMM_WORLD,&globalSize); 

	int a[N], b[N], c[N]; // arrays 

	// make communicator between the ioProcess and compProcess
	MPI_Comm newComm; 
	colour = globalRank%(globalSize/2); // ioProc and compProc has same colour
	MPI_Comm_split(MPI_COMM_WORLD, colour, globalRank, &newComm); 
	int newRank; 
	MPI_Comm_rank(newComm,&newRank); 
	printf("%i global rank, %i new rank, %i colour \n", globalRank, newRank, colour); 


	// timers 
	double start, stop, diff; 

	start = MPI_Wtime();

	for(int i = 0; i < 3; i++)
	{
		int* array; 
		switch(i){
			case 0:
				array = a; 
				break ; 
			case 1:
				array = b; 
				break ; 
			case 2:
				array = c; 
				break ; 
		} 

		// new window in new communicator to connect both processes 
		MPI_Win win; 
		int soi = sizeof(int);
		ierr = MPI_Win_create(array, soi*N, soi, MPI_INFO_NULL, newComm, &win);
		if(ierr!=MPI_SUCCESS)
		{
			printf("errors \n"); 
		}

		// initialise variables if process is comp process
		// MPI_Win_fence(0, win); // open fence 
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, win); 
		if(newRank == 0) // comp process 
		{
			initialise(array, i); 
		} 
		// MPI_Win_fence(0, win); // close fence 
		MPI_Win_unlock(0, win); 

		// read variables if process is io process 
		// MPI_Win_fence(0, win); // open fence 
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 1, 0, win); 
		if(newRank == 1)
		{
			int targetRank = 0; 
			MPI_Get(array, N, MPI_INT, targetRank, 0, N, MPI_INT, win);
		}
		MPI_Win_unlock(1, win); 
		// MPI_Win_fence(0, win); // close fence 
		MPI_Win_free(&win);
		
		// print values 
		if(newRank == 1)
		{
			printf("Global rank %i reading array with value set to be %i \n",globalRank, i); 
			printData(array); 
		} 
	}

	stop = MPI_Wtime();
	diff = stop - start;

	printf("Rank %d, took %8.8fs\n",
			globalRank,  diff);

	MPI_Finalize();
	return 0;
} 

