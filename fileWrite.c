#include "stream_post_ioserver.h"
void fileWrite(struct params *ioParams, double* iodata, int* loopCounter, int windowNum)
{	
#ifdef IOBW
	ioParams->writeTime[windowNum][loopCounter[windowNum]] = MPI_Wtime(); 
#endif 
	mpiiowrite(iodata, ioParams->arraysubsize, ioParams->arraygsize, ioParams->arraystart, NDIM, ioParams->cartcomm, ioParams->WRITEFILE[windowNum]); 
	// phdf5write(iodata, ioParams->arraysubsize, ioParams->arraygsize, ioParams->arraystart, NDIM, ioParams->cartcomm, ioParams->WRITEFILE[windowNum]); 
#ifdef IOBW
	// finish write time 
	ioParams->writeTime[windowNum][loopCounter[windowNum]] = MPI_Wtime() - ioParams->writeTime[windowNum][loopCounter[windowNum]]; 
	// finish winTime 
	ioParams->winTime[windowNum][loopCounter[windowNum]] = MPI_Wtime() - ioParams->winTime[windowNum][loopCounter[windowNum]];   
#endif 
	// phdf5write(iodata, arraysubsize, arraygsize, arraystart, NDIM, cartcomm, FILENAME); 
	// mpiRead(FILENAME, ioComm); 
} 
