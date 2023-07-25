#include "stream_post_ioserver.h"
void fileWrite(double* iodata, int*arraysubsize, int* arraygsize, int* arraystart, int ndim, MPI_Comm cartcomm, char* FILENAME, MPI_Comm ioComm)
{

	mpiiowrite(iodata, arraysubsize, arraygsize, arraystart, NDIM, cartcomm, FILENAME); 
	// phdf5write(iodata, arraysubsize, arraygsize, arraystart, NDIM, cartcomm, FILENAME); 
	mpiRead(FILENAME, ioComm); 
} 
