#include "sharedmem.h"
#include <errno.h>
void fileWrite(struct params *ioParams, double* iodata, int* loopCounter, int windowNum)
{	
#ifdef IOBW
	ioParams->writeTime_start = MPI_Wtime(); 
#ifndef NDEBUG 
	fprintf(ioParams->debug, "fileWrite->Before writing window:%i loopCounter %i, writeTime %lf, winTime %lf \n",windowNum, 
	loopCounter[windowNum], ioParams->writeTime[windowNum][loopCounter[windowNum]], ioParams->winTime[windowNum][loopCounter[windowNum]]); 
#endif 
#endif
	
#ifndef NDEBUG
		int ioRank; 
		MPI_Comm_rank(ioParams->ioComm, &ioRank); 
    fprintf(ioParams->debug, "fileWrite-> IO rank %i Writing to filename = %s loopCounter %i WindowNum %i\n", ioRank, ioParams->WRITEFILE[windowNum][loopCounter[windowNum]], loopCounter[windowNum], windowNum);
#endif 

	printf("Before calling adios2 \n"); 

	
	// call io libraries 
	switch(ioParams->ioLibNum)
	{
		case(0): 
			mpiiowrite(iodata,ioParams->WRITEFILE[windowNum][loopCounter[windowNum]], ioParams); 
			break; 
		case(1): 
			phdf5write(iodata,ioParams->WRITEFILE[windowNum][loopCounter[windowNum]], ioParams); 
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
	fprintf(ioParams->debug, "fileWrite->After writing window:%i loopCounter %i, writeTime %lf, winTime %lf \n", windowNum, 
	loopCounter[windowNum], ioParams->writeTime[windowNum][loopCounter[windowNum]], ioParams->winTime[windowNum][loopCounter[windowNum]]); 
#endif 
#endif 
	// increment loopCounter after filewrite complete 
	loopCounter[windowNum]++; 
} 
