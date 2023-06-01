#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#define N 3 
int main(int argc, char** argv)
{
	//skipped initialization above
	MPI_Init(&argc, &argv);
	MPI_Status status; 
	int ierr, i; 
	int ranks[3];  
	MPI_Win win;

	int rank; 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 

	//Start up MPI...
	MPI_Group comm_group, group;

	for (i=0;i<3;i++) {
		ranks[i] = i;     //For forming groups, later
	}
	MPI_Comm_group(MPI_COMM_WORLD,&comm_group);

	// allocate memory
	int soi = sizeof(int);
	int* buf = (int *)malloc(soi*N); 
	buf[0]=rank; 

	/* Create new window for this comm */
	if (rank == 0) {
		MPI_Win_create(buf,sizeof(int)*3,sizeof(int),
				MPI_INFO_NULL,MPI_COMM_WORLD,&win);
	}
	else {
		/* Rank 1 or 2 */
		MPI_Win_create(NULL,0,sizeof(int),
				MPI_INFO_NULL,MPI_COMM_WORLD,&win);
	}
	/* Now do the communication epochs */
	if (rank == 0) {
		/* Origin group consists of ranks 1 and 2 */
		MPI_Group_incl(comm_group,2,ranks+1,&group);
		/* Begin the exposure epoch */
		MPI_Win_post(group,0,win);
		/* Wait for epoch to end */
		MPI_Win_wait(win);
	}
	else {
		/* Target group consists of rank 0 */
		MPI_Group_incl(comm_group,1,ranks,&group);
		/* Begin the access epoch */
		MPI_Win_start(group,0,win);
		/* Put into rank==0 according to my rank */
		MPI_Put(buf,1,MPI_INT,0,rank,1,MPI_INT,win);
		/* Terminate the access epoch */
		MPI_Win_complete(win);
	}
	
	if(rank == 0)
	{
		printf("Value of array fron rank 0 \n"); 
		for(int i = 0; i < 3; i++)
		{
			printf("%i, ", buf[i]); 
		}
		printf("\n"); 
	}
	/* Free window and groups */
	MPI_Win_free(&win);
	MPI_Group_free(&group);
	MPI_Group_free(&comm_group);

	//Shut down...
	MPI_Finalize();
	free(buf); 
	buf = NULL; 

	return 0;
} 
