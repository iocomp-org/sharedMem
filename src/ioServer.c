/*
 * function gets shared array pointer using mpi win shared query 
 * and prints out data as proxy for writing to file 
 */

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include <string.h> 
#include <assert.h> 
#include <math.h>
#include "sharedmem.h"

void ioServer(MPI_Comm ioComm, MPI_Comm newComm, struct params *ioParams)
{

	// allocate windows 
	double* array[NUM_WIN]; 
	int soi = sizeof(double); 

	// initialise IO Params structure 
	ioParams->ioComm = ioComm; 
	int ioRank, ierr; 
	ierr = MPI_Comm_rank(ioParams->ioComm, &ioRank); 
	error_check(ierr); 

	int loopCounter[NUM_WIN]; 

	// Allocate cartesian communicator, adios2 objects	
	ioServerInitialise(ioParams); 

	// Initialise array parameters for each process write into a global file  
	arrayParamsInit(ioParams); 

	// allocate shared windows 
	for(int i = 0; i < NUM_WIN; i++)
	{
		ierr = MPI_Win_allocate_shared(0, soi, MPI_INFO_NULL, newComm, &array[i], &ioParams->win_ptr[i]); 
		error_check(ierr);
#ifndef NDEBUG 
		fprintf(ioParams->debug, "ioServer -> MPI allocatedioParams-> windows %i \n", i); 
#endif 
	} 

	// allocate arrays using window pointers 
	for(int i = 0; i < NUM_WIN; i++)
	{
		long int arraySize; 
		int dispUnit; 
		ierr = MPI_Win_shared_query(ioParams->win_ptr[i], 0, &arraySize, &dispUnit, &array[i]); 
		error_check(ierr); 
#ifndef NDEBUG 
		fprintf(ioParams->debug, "ioServer -> MPI shared query %i \n", i); 
#endif 
	} 

	// groups 
	MPI_Group comm_group, group;
	int ranks[2]; 
	for (int j=0; j<2; j++) 
	{
		ranks[j] = j;   
	}
	MPI_Comm_group(newComm,&comm_group);

	/* Compute group consists of rank 0*/
	MPI_Group_incl(comm_group,1,ranks,&group); 
	// assign wintestflags int to test for messages from the compute server  
	int wintestflags[NUM_WIN]; 
	// declare mult variable to test for completion among all windows 
	int wintestmult = 1; 

	// loopCounter to assign timers per loop iteration for each window
	for(int i = 0 ; i < NUM_WIN; i ++)
	{
		loopCounter[i] = 0; 
#ifdef IOBW
		// initialise timers 
		for(int j = 0; j < AVGLOOPCOUNT; j++)
		{
			ioParams->winTime[i][j] = 0.0; 
			ioParams->writeTime[i][j] = 0.0; 
		}
#endif 
	}
	
	for(int i = 0; i< NUM_WIN; i++)
	{
		ioParams->flagReturn[i] = 0; 
		ioParams->writeComplete[i] = 0; 
	} 

	/* 
	 * Do while loop to check the status of each window, and different actions
	 * based on the window control array parameters 
	 */ 
	do 
	{
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
		fprintf(ioParams->debug, "ioServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
#endif 

		/* 
		 * iterate across all windows 
		 */ 
		for(int i = 0; i < NUM_WIN; i++)
		{
			if(wintestflags[i] > WIN_DEACTIVATE) // anything over 0 means go for printing 
			{
				if(wintestflags[i]==WIN_WAIT && ioParams->writeComplete[i]==0)  
				{
					/* 
					 * if wait activated BUT MPI win test has returned a non successful
					 * value. After call returns call file write. 
					 */ 
#ifndef NDEBUG 
					fprintf(ioParams->debug, "ioServer window:%i flag negative and win wait implemented\n", i); 
#endif 
					ierr = MPI_Win_wait(ioParams->win_ptr[i]); 
					error_check(ierr); 
					fileWrite(ioParams, array[i], loopCounter, i); 
					ioParams->writeComplete[i] = 1; 
				}
				else if(wintestflags[i]==WIN_TEST && ioParams->writeComplete[i]==0) 
				{
					/* 
					 * if WIN TEST flag is passed, and file has not been written out, call MPI win test, 
					 * and if successful then call file write 
					 */ 
					winTest(ioParams,array[i], i, loopCounter); 
				}
				
				if(wintestflags[i]==WIN_ACTIVATE || ioParams->writeComplete[i]==1)
				{
					/*
					 * if previously written, or newly activated window then start sync
					 * process again with a call to MPI win post
					 */ 
					ierr = MPI_Win_post(group, 0, ioParams->win_ptr[i]);
					error_check(ierr); 
#ifndef NDEBUG 
					fprintf(ioParams->debug, "ioServer window:%i MPI post loopCounter %i\n", i, loopCounter[i]); 
#endif 
#ifdef IOBW	
					ioParams->winTime_start[i] = MPI_Wtime();
#endif 
					winTest(ioParams, array[i], i, loopCounter); 
				} 
			}
		} 

		/*
		 * Check if any windows are going to be freed. 
		 * if the control array has the WIN FREE constant, then break the loop. 
		 */ 
		wintestmult = 0;  // reset value 
		for(int i = 0; i < NUM_WIN; i++)
		{
			if(wintestflags[i] == WIN_FREE)
			{
				wintestmult = 1; 
			}
		} 
#ifndef NDEBUG 
		fprintf(ioParams->debug, "ioServer -> wintestmult value %i\n", wintestmult); 
#endif 
	}while(!wintestmult);  // test for completion of all windows 

#ifndef NDEBUG 
	fprintf(ioParams->debug, "ioServer -> loop server exited \n"); 
#endif 

	// free windows and pointer 
	// while freeing, check if there are any opened windows 
	for(int i = 0; i < NUM_WIN; i++)
	{
		// wait for completion of all windows 
		ierr = MPI_Win_wait(ioParams->win_ptr[i]); 
		error_check(ierr); 
		fileWrite(ioParams, array[i], loopCounter, i); 
#ifndef NDEBUG 
		fprintf(ioParams->debug, "ioServer window:%i win wait reached\n",i); 
#endif 
		ierr = MPI_Win_free(&ioParams->win_ptr[i]);
		error_check(ierr); 
	} 

	// Finalise ADIOS2 object
	if(ioParams->ioLibNum >=2 && ioParams->ioLibNum <= 4)
	{
		adios2_finalize(ioParams->adios);
#ifndef NDEBUG 
		fprintf(ioParams->debug, "ioServer->adios2 finalised\n"); 
#endif	
	} 

#ifdef IOBW
	iobw(ioParams); 
#endif 

#ifdef VERIFY
	if(!ioRank)
	{
		printf("Verification started \n"); 
	}
	verify(ioParams); 
#endif

#ifndef NODELETE
	MPI_Barrier(ioParams->ioComm); 
	if(!ioRank)
	{
		deleteFiles(ioParams); 
#ifndef NDEBUG 
		fprintf(ioParams->debug, "ioServer->file/directory deleted \n"); 
#endif	
	} 
#endif 
} 
