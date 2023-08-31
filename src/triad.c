#include "sharedmem.h"
void triad(struct params *ioParams, int iter, MPI_Comm newComm, MPI_Win win_A, MPI_Group group, double* a, double *b, double *c)
{
	// TRIAD 
	// send message to ioServer to complete A
	// then update A
	// send message to ioServer to print via broadcast
	if(iter > 0)
	{
		ioParams->wintestflags[WIN_A] = WIN_WAIT;  
		ioParams->wintestflags[WIN_C] = WIN_TEST; 
		ioParams->wintestflags[WIN_B] = WIN_TEST; 
	}
	else
	{
		ioParams->wintestflags[WIN_A] = WIN_ACTIVATE;  
		ioParams->wintestflags[WIN_C] = WIN_TEST; 
		ioParams->wintestflags[WIN_B] = WIN_TEST; 
	}
	MPI_Bcast( ioParams->wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
	fprintf(ioParams->debug, "compServer -> after MPI bcast, ioParams->wintestflags [%i,%i,%i] \n", ioParams->wintestflags[0], ioParams->wintestflags[1], ioParams->wintestflags[2]); 
#endif 

	ioParams->compTimer[TRIAD][iter] = MPI_Wtime(); 
	MPI_Win_start(group, 0, win_A); 
#ifndef NDEBUG 
	fprintf(ioParams->debug, "compServer -> After mpi start for A \n"); 
#endif 

	for(int i = 0; i < ioParams->localDataSize; i++)
	{
		a[i] = b[i] + SCALAR*c[i]; 
	}

	MPI_Win_complete(win_A); 
	ioParams->compTimer[TRIAD][iter] = MPI_Wtime() - ioParams->compTimer[TRIAD][iter]; 
#ifndef NDEBUG 
	fprintf(ioParams->debug, "compServer -> After mpi complete for A \n"); 
#endif 
}
