#include "stream_post_ioserver.h"
void fileWrite(struct params *ioParams, double* iodata, int* loopCounter, int windowNum)
{	
#ifdef IOBW
	ioParams->writeTime_start = MPI_Wtime(); 
#ifndef NDEBUG 
	printf("fileWrite->Before writing window:%i loopCounter %i, writeTime %lf, winTime %lf \n",windowNum, 
	loopCounter[windowNum], ioParams->writeTime[windowNum][loopCounter[windowNum]], ioParams->winTime[windowNum][loopCounter[windowNum]]); 
#endif 
#endif 
	
	// call io libraries 
	mpiiowrite(iodata, ioParams->arraysubsize, ioParams->arraygsize, ioParams->arraystart, NDIM, ioParams->cartcomm, ioParams->WRITEFILE[windowNum]); 
	phdf5write(iodata, ioParams->arraysubsize, ioParams->arraygsize, ioParams->arraystart, NDIM, ioParams->cartcomm, ioParams->WRITEFILE[windowNum]); 

#ifdef IOBW
	// finish write time 
	ioParams->writeTime_end = MPI_Wtime(); 
	ioParams->writeTime[windowNum][loopCounter[windowNum]] = ioParams->writeTime_end - ioParams->writeTime_start; 
	// finish window, and end win timer 
	ioParams->winTime_end[windowNum] = MPI_Wtime(); 
	ioParams->winTime[windowNum][loopCounter[windowNum]] = ioParams->winTime_end[windowNum] - ioParams->winTime_start[windowNum]; 
	// increment loopCounter after filewrite complete 
	loopCounter[windowNum]++; 
#ifndef NDEBUG 
	printf("fileWrite->After writing window:%i loopCounter %i, writeTime %lf, winTime %lf \n", windowNum, 
	loopCounter[windowNum], ioParams->writeTime[windowNum][loopCounter[windowNum]], ioParams->winTime[windowNum][loopCounter[windowNum]]); 
#endif 
#endif 
} 
