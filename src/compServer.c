#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include <string.h> 
#include <assert.h> 
#include "sharedmem.h"


void compServer(MPI_Comm computeComm, MPI_Comm newComm, MPI_Comm globalComm, struct params *ioParams)
{

	// global ranks and sizes 
	int globalRank, globalSize;
	MPI_Comm_rank(globalComm, &globalRank); 
	MPI_Comm_size(globalComm, &globalSize); 
	
	// compute ranks and sizes 
	int computeRank, computeSize; 
	MPI_Comm_rank(computeComm, &computeRank); 
	MPI_Comm_size(computeComm, &computeSize); 

	// initialise window test flags with 0 
	// assign them 1 if ready for writing 
	for(int j = 0; j < NUM_WIN; j++)
	{
		ioParams->wintestflags[j] = 0; 
	} 

	// declare and initialise timers for NUM WIN = 3 and start wall Time
	double wallTime = MPI_Wtime(); 

	for(int i = 0; i < NUM_KERNELS; i++)
	{
		for(int j = 0; j < AVGLOOPCOUNT; j++)
		{
			ioParams->compTimer[i][j] = 0.0; 
		}
	}

	// Allocate windows in order 
	MPI_Win win_A, win_B, win_C;
	double* a; 
	double* b; 
	double* c;  
	struct winElements outputWin; 
	// a array 
	outputWin = winAlloc(ioParams->localDataSize, newComm, ioParams); 
	win_A = outputWin.win; 
	a = outputWin.array; 

	// c array 
	outputWin = winAlloc(ioParams->localDataSize, newComm, ioParams); 
	win_C = outputWin.win; 
	c = outputWin.array; 

	// b array 
	outputWin = winAlloc(ioParams->localDataSize, newComm, ioParams); 
	win_B = outputWin.win; 
	b = outputWin.array; 

	// groups newComm communicator's rank 0 and 1 into a group  
	// comp process initialises array and creates a window with that array 
	MPI_Group comm_group, group;
	int ranks[2]; 
	for (int i=0;i<2;i++) {
		ranks[i] = i;     //For forming groups, later
	}
	MPI_Comm_group(newComm,&comm_group);

	/* I/O group consists of ranks 1*/
	MPI_Group_incl(comm_group,1,ranks+1,&group); 

	// INITIALISE A
	// send message to ioServer to print via broadcast
//	ioParams->wintestflags[WIN_A] = WIN_ACTIVATE;  
//	ioParams->wintestflags[WIN_C] = WIN_DEACTIVATE; 
//	ioParams->wintestflags[WIN_B] = WIN_DEACTIVATE; 
//	MPI_Bcast( ioParams->wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
//#ifndef NDEBUG 
//	printf("compServer -> after MPI bcast, ioParams->wintestflags [%i,%i,%i] \n", ioParams->wintestflags[0], ioParams->wintestflags[1], ioParams->wintestflags[2]); 
//#endif 
//	MPI_Win_start(group, 0, win_A); 
//#ifndef NDEBUG 
//	printf("compServer -> MPI window start with global rank %i \n", globalRank); 
//#endif 
//	for(int i = 0; i < ioParams->localDataSize; i++)
//	{
//		// a[i] = i + ((globalRank)*ioParams->localDataSize); 
//		a[i] = STARTING_VAL; 
//	}
//	MPI_Win_complete(win_A);
//		if(!computeRank)
//		{
//			printf("value of A = %lf \n", a[0]); 
//		}
//#ifndef NDEBUG 
//	printf("compServer -> After mpi window unlock for A \n"); 
//#endif 
	
	// initialise A, B, C
	for(int i = 0; i < ioParams->localDataSize; i++)
	{
		a[i] = 1.0; 
		b[i] = 2.0; 
		c[i] = 0.0; 
	}

	/* Start STREAM loop */ 
	for(int iter = 0; iter < AVGLOOPCOUNT; iter++)
	{
#ifndef NDEBUG 
		fprintf(ioParams->debug, "compServer -> LOOP number %i \n", iter+1); 
#endif 
//		// COPY
//		// send message to ioServer to print via broadcast
//		ioParams->wintestflags[WIN_A] = WIN_DEACTIVATE;  
//		// wait for C window from previous loop
//		if(iter > 0)
//		{
//			ioParams->wintestflags[WIN_C] = WIN_WAIT; 
//		}
//		else
//		{
//			ioParams->wintestflags[WIN_C] = WIN_ACTIVATE; 
//		}
//		ioParams->wintestflags[WIN_B] = WIN_DEACTIVATE; 
//		MPI_Bcast( ioParams->wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
//#ifndef NDEBUG 
//		printf("compServer -> after MPI bcast, ioParams->wintestflags [%i,%i,%i] \n", ioParams->wintestflags[0], ioParams->wintestflags[1], ioParams->wintestflags[2]); 
//#endif 
//
//		compTimer[COPY][iter] = MPI_Wtime();  
//		MPI_Win_start(group, 0, win_C); 
//#ifndef NDEBUG 
//		printf("compServer -> After win start for C \n"); 
//#endif 
//
//		for(int i = 0; i < ioParams->localDataSize; i++)
//		{
//			c[i] = a[i]; 
//		}
//
//		MPI_Win_complete(win_C); 
//		compTimer[COPY][iter] = MPI_Wtime() - compTimer[COPY][iter]; 
//#ifndef NDEBUG 
//		printf("compServer -> After mpi window unlock for C \n"); 
//#endif 

		// SCALE
		// send message to ioServer to print via broadcast
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
		
		/*
		 * update control variable, WIN_WAIT/ACTIVATE for C
		 * ADD kernel C = A+B
		 */ 
		add(ioParams,iter,newComm,win_C,group, a,b,c); 

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

	// send message to ioServer to free the windows and exit the recv loop
	ioParams->wintestflags[WIN_A] = WIN_FREE;  
	ioParams->wintestflags[WIN_C] = WIN_FREE; 
	ioParams->wintestflags[WIN_B] = WIN_FREE; 
	MPI_Bcast( ioParams->wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
	fprintf(ioParams->debug, "compServer -> after MPI bcast, ioParams->wintestflags [%i,%i,%i] \n", ioParams->wintestflags[0], ioParams->wintestflags[1], ioParams->wintestflags[2]); 
#endif 

	// MPI_Barrier(MPI_COMM_WORLD); 	
	dataSendComplete(win_A,ioParams); 
	dataSendComplete(win_C,ioParams); 
	dataSendComplete(win_B,ioParams); 

	wallTime = MPI_Wtime() - wallTime; 

	MPI_Reduce(&wallTime,&ioParams->wallTime_max,1, MPI_DOUBLE, MPI_MAX, 0,computeComm); 

	for(int i = 0; i < NUM_KERNELS; i++)
	{
		MPI_Reduce(&ioParams->compTimer[i],&ioParams->compTimer_max[i],AVGLOOPCOUNT, MPI_DOUBLE, MPI_MAX, 0,computeComm); 
		MPI_Reduce(&ioParams->compTimer[i],&ioParams->compTimer_min[i],AVGLOOPCOUNT, MPI_DOUBLE, MPI_MIN, 0,computeComm); 
	}
	
	// print out the different timers, along with GB/s to output file 
	if(!computeRank)
	{
		compPrints(ioParams); 
	}
} 
