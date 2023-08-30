#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include <string.h> 
#include <assert.h> 
#include <math.h>
#include "sharedmem.h"

void winTest(struct params *ioParams,double* array, int windowNum, int* loopCounter)
{
	int ierr; 
	// test for window completion 	
	ierr = MPI_Win_test(ioParams->win_ptr[windowNum], &ioParams->flagReturn[windowNum]); 
	error_check(ierr);
#ifndef NDEBUG 
	fprintf(ioParams->debug, "ioServer window:%i win test\n",windowNum); 
#endif 

	// if WINDOW ready to be written:  
	if(ioParams->flagReturn[windowNum])
	{
		printf("window %i, flag is positive going for writing \n", windowNum); 
#ifndef NDEBUG 
		fprintf(ioParams->debug, "ioServer window:%i flag positive \n",windowNum); 
#endif
		fileWrite(ioParams, array, loopCounter, windowNum); 
		printf("window %i, after filewrite \n", windowNum); 
		ioParams->writeComplete[windowNum] = 1; 
	}

} 
