#include "sharedmem.h"
void scale(struct params *ioParams, int iter, MPI_Comm newComm, MPI_Win win_B, MPI_Group group, double *b, double *c)
{
	if(iter > 0)
	{
		ioParams->wintestflags[WIN_A] = WIN_TEST;  
		ioParams->wintestflags[WIN_C] = WIN_TEST; 
		ioParams->wintestflags[WIN_B] = WIN_WAIT; 
	}
	else
	{
		ioParams->wintestflags[WIN_A] = WIN_DEACTIVATE;  
		ioParams->wintestflags[WIN_C] = WIN_DEACTIVATE; 
		ioParams->wintestflags[WIN_B] = WIN_ACTIVATE; 
	}
	MPI_Bcast( ioParams->wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
	fprintf(ioParams->debug, "compServer -> after MPI bcast, ioParams->wintestflags [%i,%i,%i] \n", ioParams->wintestflags[0], ioParams->wintestflags[1], ioParams->wintestflags[2]); 
#endif 

	ioParams->compTimer[SCALE][iter] = MPI_Wtime(); 
	MPI_Win_start(group, 0, win_B); 
#ifndef NDEBUG 
	fprintf(ioParams->debug, "compServer -> After win start for B\n"); 
#endif 

	for(int i = 0; i < ioParams->localDataSize; i++)
	{
		b[i] = SCALAR * c[i]; 
	}

	MPI_Win_complete(win_B); 
	ioParams->compTimer[SCALE][iter] = MPI_Wtime() - ioParams->compTimer[SCALE][iter]; 
#ifndef NDEBUG 
	fprintf(ioParams->debug, "compServer -> After mpi window unlock for B \n"); 
#endif 

} 
