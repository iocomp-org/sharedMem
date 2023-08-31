#include "sharedmem.h"
void copy(struct params *ioParams, int iter, MPI_Comm newComm, MPI_Win win_C, MPI_Group group, double* a, double *c)
{
	// COPY
	// send message to ioServer to print via broadcast
	if(iter > 0)
	{
		ioParams->wintestflags[WIN_A] = WIN_TEST;  
		ioParams->wintestflags[WIN_C] = WIN_WAIT; 
		ioParams->wintestflags[WIN_B] = WIN_TEST; 
	}
	else
	{
		ioParams->wintestflags[WIN_A] = WIN_DEACTIVATE;  
		ioParams->wintestflags[WIN_C] = WIN_ACTIVATE; 
		ioParams->wintestflags[WIN_B] = WIN_DEACTIVATE; 
	}
	MPI_Bcast( ioParams->wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
	printf("compServer -> after MPI bcast, ioParams->wintestflags [%i,%i,%i] \n", ioParams->wintestflags[0], ioParams->wintestflags[1], ioParams->wintestflags[2]); 
#endif 

	ioParams->compTimer[COPY][iter] = MPI_Wtime();  
	MPI_Win_start(group, 0, win_C); 
#ifndef NDEBUG 
	printf("compServer -> After win start for C \n"); 
#endif 

	/* outer loop to increase compute workload */ 
	for(int j = 0; j < COMPLOOPCOUNT; j++)
	{
		for(int i = 0; i < ioParams->localDataSize; i++)
		{
			c[i] = a[i]; 
		}
	} 

	MPI_Win_complete(win_C); 
	ioParams->compTimer[COPY][iter] = MPI_Wtime() - ioParams->compTimer[COPY][iter]; 
#ifndef NDEBUG 
	printf("compServer -> After mpi window unlock for C \n"); 
#endif 
} 
