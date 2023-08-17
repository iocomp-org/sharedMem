#include "sharedmem.h"
#include <errno.h>
void fileWrite(struct params *ioParams, double* iodata, int* loopCounter, int windowNum)
{	
#ifdef IOBW
	ioParams->writeTime_start = MPI_Wtime(); 
#ifndef NDEBUG 
	printf("fileWrite->Before writing window:%i loopCounter %i, writeTime %lf, winTime %lf \n",windowNum, 
	loopCounter[windowNum], ioParams->writeTime[windowNum][loopCounter[windowNum]], ioParams->winTime[windowNum][loopCounter[windowNum]]); 
#endif 
#endif
	
#ifndef NDEBUG
		int ioRank; 
		MPI_Comm_rank(ioParams->ioComm, &ioRank); 
    fprintf(ioParams->debug, "fileWrite-> IO rank %i Writing to filename = %s loopCounter %i WindowNum %i\n", ioRank, ioParams->WRITEFILE[windowNum][loopCounter[windowNum]], loopCounter[windowNum], windowNum);
#endif 

	// convert size_t array parameters to int arrays using MPI and HDF5 
	int localArray[NDIM]; 
	int globalArray[NDIM]; 
	int arrayStart[NDIM]; 
	for(int i = 0; i < NDIM; i++)
	{
		localArray[i] = (int)ioParams->localArray[i]; 
		globalArray[i] = (int)ioParams->globalArray[i]; 
		arrayStart[i] = (int)ioParams->arrayStart[i]; 
	}
	
	// call io libraries 
	switch(ioParams->ioLibNum)
	{
		case(0): 
			mpiiowrite(iodata, localArray, globalArray, arrayStart, ioParams->cartcomm, ioParams->WRITEFILE[windowNum][loopCounter[windowNum]], ioParams); 
			break; 
		case(1): 
			phdf5write(iodata, localArray, globalArray, arrayStart, ioParams->cartcomm, ioParams->WRITEFILE[windowNum][loopCounter[windowNum]], ioParams); 
			break; 
		case(2): case(3): case(4): 
			adioswrite(iodata, ioParams->WRITEFILE[windowNum][loopCounter[windowNum]], ioParams); 
			break; 
		default:
			printf("Invalid io number"); 
			break; 
	} 

#ifdef IOBW
	// finish write time 
	ioParams->writeTime_end = MPI_Wtime(); 
	ioParams->writeTime[windowNum][loopCounter[windowNum]] = ioParams->writeTime_end - ioParams->writeTime_start; 
	// finish window, and end win timer 
	ioParams->winTime_end[windowNum] = MPI_Wtime(); 
	ioParams->winTime[windowNum][loopCounter[windowNum]] = ioParams->winTime_end[windowNum] - ioParams->winTime_start[windowNum]; 
#ifndef NDEBUG 
	printf("fileWrite->After writing window:%i loopCounter %i, writeTime %lf, winTime %lf \n", windowNum, 
	loopCounter[windowNum], ioParams->writeTime[windowNum][loopCounter[windowNum]], ioParams->winTime[windowNum][loopCounter[windowNum]]); 
#endif 
#endif 
	// increment loopCounter after filewrite complete 
	loopCounter[windowNum]++; 
} 
