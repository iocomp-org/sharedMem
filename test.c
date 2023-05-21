#include <stdio.h>
#include <mpi.h>
#define N 10 

// function to send message using MPI one sided communication 
void readData(int* recv, int targetRank, int myRank, MPI_Comm ioComm)
{
	// create mpi window 
	MPI_Win win; 
	int soi = sizeof(int);
	MPI_Win_create(recv, soi*N, soi, MPI_INFO_NULL, ioComm, &win);
	printf("MPI window created by rank %i \n", myRank); 
	
	// read data after fencing  
	MPI_Win_fence(0, win);
	MPI_Get(recv, N, MPI_INT, targetRank, 0, N, MPI_INT, win);
	// Sync to make sure the get is complete
	MPI_Win_fence(0, win);
	
	// free data 
	MPI_Win_free(&win);
	
	// print data 
	printf("Reading data \n");
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

	int myRank, mySize, colour; 
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank); 
	MPI_Comm_size(MPI_COMM_WORLD,&mySize); 

	int a[N], b[N], c[N]; // arrays 

	// initialise arrays if process is computeServer
	if(myRank < mySize/2)
	{
		colour = 0;
		MPI_Comm compComm; 
		MPI_Comm_split(MPI_COMM_WORLD, colour, myRank, &compComm); // split comm
		printf("Initialisation by rank %i \n", myRank); 
		for(int j = 0; j < N; j++)
		{	
			a[j] = 1; 
			b[j] = 2; 
			c[j] = 3; 
		} 
	} 
	
	// read arrays if process is ioServer
	else
	{
		// split communicator to get ioComm
		colour = 1; 
		MPI_Comm ioComm; 
		MPI_Comm_split(MPI_COMM_WORLD, colour, myRank, &ioComm); 

		double start = MPI_Wtime();

		for(int i=0; i<mySize/2; i++)
		{
			int targetRank = myRank%(mySize/2); // pair with correct rank from compute process
			printf("Reading of array by rank %i \n", myRank); 
			readData(a, targetRank, myRank, ioComm); // read array 

		}
			
		double stop = MPI_Wtime();
		double diff = stop - start;
		printf("Rank %d, %d-double transactions took %8.8fs\n",
			myRank, mySize, diff);
	} 

	MPI_Finalize();
	return 0;
}
