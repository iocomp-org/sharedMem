#include <stdlib.h>  
#include <string.h> 
#include <stdio.h>
#include <mpi.h>
#include "sharedmem.h"
#define FILENAME "ioserver_output.csv"

void iobw(struct params *ioParams)
{
	// print out timers by reducing all the variables to get the maximum value 
	//
	int ioRank, ierr; 
	ierr = MPI_Comm_rank(ioParams->ioComm, &ioRank); 
	error_check(ierr); 

	// MPI reduction of writeTime array over all IO ranks 
	for(int i = 0; i < NUM_WIN; i++)
	{
		ierr = MPI_Reduce(ioParams->writeTime[i], ioParams->writeTime_max[i], AVGLOOPCOUNT, MPI_DOUBLE, MPI_MAX, 0, ioParams->ioComm); 
		error_check(ierr); 
		ierr = MPI_Reduce(ioParams->winTime[i], ioParams->winTime_max[i], AVGLOOPCOUNT, MPI_DOUBLE, MPI_MAX, 0, ioParams->ioComm); 
		error_check(ierr); 
	}

	// calculate file size for B/W calculation 
	ioParams->fileSize = sizeof(double); 
	for(int i = 0; i < NDIM; i++)
	{
		ioParams->fileSize *= ioParams->localArray[i]; 
	}

	// ioRank = 0 writes stats to output file 
	if(!ioRank)
	{
    // initialise and declare file object 
    FILE* out;
    
		remove(FILENAME);
		out = fopen(FILENAME, "w+");
		if (out == NULL)
		{
			printf("Error: No output file\n");
			exit(1);
		}
		// header for print statements
		fprintf(out,"Loop_number,Window_Number,Window_Time(s),Write_Time(s),IO_BW(GB/s)\n"); 
		for(int j = 0; j < AVGLOOPCOUNT; j ++)
		{
			for(int i = 0; i < NUM_WIN; i++)
			{
				if(ioParams->writeTime_max[i][j] > 0.0)
				{
					fprintf(out,"%i,%i,%.3f,%.3f,%.3f \n",j,i,ioParams->winTime_max[i][j],ioParams->writeTime_max[i][j],
							(1.0E-09*ioParams->fileSize/ioParams->writeTime_max[i][j]) ); 
				} 
			} 
		} 
#ifndef NDEBUG 
		fprintf(ioParams->debug, "ioServer->Stats written \n",i); 
#endif 
		fclose(out); 	
	}
} 
