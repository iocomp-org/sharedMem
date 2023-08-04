#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include <string.h> 
#include <assert.h> 
#include "stream_post_ioserver.h"

#define FILENAME "compserver_output.csv"

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
	int wintestflags[NUM_WIN]; 	
	for(int j = 0; j < NUM_WIN; j++)
	{
		wintestflags[j] = 0; 
	} 

	// declare and initialise timers for NUM WIN = 3 and start wall Time
	double compTimer[NUM_KERNELS][AVGLOOPCOUNT]; 
	double waitTimer[NUM_KERNELS][AVGLOOPCOUNT]; 
	double wallTime = MPI_Wtime(); 

	FILE* out; 

	for(int i = 0; i < NUM_KERNELS; i++)
	{
		for(int j = 0; j < AVGLOOPCOUNT; j++)
		{
			compTimer[i][j] = 0.0; 
			waitTimer[i][j] = 0.0; 
		}
	}

	// Allocate windows in order 
	MPI_Win win_A, win_B, win_C;
	double* a; 
	double* b; 
	double* c;  
	struct winElements outputWin; 
	// a array 
	outputWin = winAlloc(ioParams->N, newComm); 
	win_A = outputWin.win; 
	a = outputWin.array; 

	// c array 
	outputWin = winAlloc(ioParams->N, newComm); 
	win_C = outputWin.win; 
	c = outputWin.array; 

	// b array 
	outputWin = winAlloc(ioParams->N, newComm); 
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
	wintestflags[WIN_A] = WIN_ACTIVATE;  
	wintestflags[WIN_C] = WIN_DEACTIVATE; 
	wintestflags[WIN_B] = WIN_DEACTIVATE; 
	MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
	printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
#endif 
	MPI_Win_start(group, 0, win_A); 
#ifndef NDEBUG 
	printf("compServer -> MPI window start with global rank %i \n", globalRank); 
#endif 
	for(int i = 0; i < ioParams->N; i++)
	{
		// a[i] = STARTING_VAL;  
		a[i] = i + ((globalRank)*ioParams->N); 
	}
	MPI_Win_complete(win_A);
#ifndef NDEBUG 
	printf("compServer -> After mpi window unlock for A \n"); 
#endif 

	/* Start STREAM loop */ 
	for(int iter = 0; iter < AVGLOOPCOUNT; iter++)
	{
#ifndef NDEBUG 
		printf("compServer -> LOOP number %i \n", iter+1); 
#endif 
		// COPY
		// send message to ioServer to print via broadcast
		wintestflags[WIN_A] = WIN_DEACTIVATE;  
		// wait for C window from previous loop
		if(iter > 0)
		{
			wintestflags[WIN_C] = WIN_WAIT; 
		}
		else
		{
			wintestflags[WIN_C] = WIN_ACTIVATE; 
		}
		wintestflags[WIN_B] = WIN_DEACTIVATE; 
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
#endif 

		compTimer[COPY][iter] = MPI_Wtime();  
		MPI_Win_start(group, 0, win_C); 
#ifndef NDEBUG 
		printf("compServer -> After win start for C \n"); 
#endif 

		for(int i = 0; i < ioParams->N; i++)
		{
			c[i] = a[i]; 
		}

		MPI_Win_complete(win_C); 
		compTimer[COPY][iter] = MPI_Wtime() - compTimer[COPY][iter]; 
#ifndef NDEBUG 
		printf("compServer -> After mpi window unlock for C \n"); 
#endif 

		// SCALE
		// send message to ioServer to print via broadcast
		wintestflags[WIN_A] = WIN_DEACTIVATE;  
		wintestflags[WIN_C] = WIN_DEACTIVATE; 
		if(iter > 0)
		{
			wintestflags[WIN_B] = WIN_WAIT; 
		}
		else
		{
			wintestflags[WIN_B] = WIN_ACTIVATE; 
		}
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
#endif 

		compTimer[SCALE][iter] = MPI_Wtime(); 
		MPI_Win_start(group, 0, win_B); 
#ifndef NDEBUG 
		printf("compServer -> After win start for B\n"); 
#endif 

		for(int i = 0; i < ioParams->N; i++)
		{
			b[i] = SCALAR * c[i]; 
		}

		MPI_Win_complete(win_B); 
		compTimer[SCALE][iter] = MPI_Wtime() - compTimer[SCALE][iter]; 
#ifndef NDEBUG 
		printf("compServer -> After mpi window unlock for B \n"); 
#endif 

		// ADD 
		// send message to ioServer to print via broadcast
		wintestflags[WIN_A] = WIN_DEACTIVATE;  
		wintestflags[WIN_C] = WIN_WAIT; 
		wintestflags[WIN_B] = WIN_DEACTIVATE; 
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
#endif 

		compTimer[ADD][iter] = MPI_Wtime(); 
		MPI_Win_start(group, 0, win_C); 
#ifndef NDEBUG 
		printf("compServer -> After win start for C \n"); 
#endif 

		for(int i = 0; i < ioParams->N; i++)
		{
			c[i] = a[i] + b[i]; 
		}

		MPI_Win_complete(win_C); 
		compTimer[ADD][iter] = MPI_Wtime() - compTimer[ADD][iter]; 
#ifndef NDEBUG 
		printf("compServer -> After mpi complete for C \n"); 
#endif 

		// TRIAD 
		// send message to ioServer to complete A
		// then update A
		// send message to ioServer to print via broadcast
		wintestflags[WIN_A] = WIN_WAIT;  
		wintestflags[WIN_C] = WIN_DEACTIVATE; 
		wintestflags[WIN_B] = WIN_DEACTIVATE; 
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
		printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
#endif 

		compTimer[TRIAD][iter] = MPI_Wtime(); 
		MPI_Win_start(group, 0, win_A); 
#ifndef NDEBUG 
		printf("compServer -> After mpi complete for A \n"); 
#endif 

		for(int i = 0; i < ioParams->N; i++)
		{
			a[i] = b[i] + SCALAR*c[i]; 
		}

		MPI_Win_complete(win_A); 
		compTimer[TRIAD][iter] = MPI_Wtime() - compTimer[TRIAD][iter]; 
#ifndef NDEBUG 
		printf("compServer -> After mpi complete for A \n"); 
#endif 
	} 

	// send message to ioServer to free the windows and exit the recv loop
	wintestflags[WIN_A] = WIN_FREE;  
	wintestflags[WIN_C] = WIN_FREE; 
	wintestflags[WIN_B] = WIN_FREE; 
	MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
	printf("compServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
#endif 

	// MPI_Barrier(MPI_COMM_WORLD); 	
	dataSendComplete(win_A); 
	dataSendComplete(win_C); 
	dataSendComplete(win_B); 

	wallTime = MPI_Wtime() - wallTime; 

	// reduction over compute time per each compute kernel 
	double compTimer_max[NUM_KERNELS][AVGLOOPCOUNT]; 
	double compTimer_min[NUM_KERNELS][AVGLOOPCOUNT]; 
	double compTimer_avg[NUM_KERNELS]; 

	double wallTime_max; 
	MPI_Reduce(&wallTime,&wallTime_max,1, MPI_DOUBLE, MPI_MAX, 0,computeComm); 

	for(int i = 0; i < NUM_KERNELS; i++)
	{
		MPI_Reduce(&compTimer[i],&compTimer_max[i],AVGLOOPCOUNT, MPI_DOUBLE, MPI_MAX, 0,computeComm); 
		MPI_Reduce(&compTimer[i],&compTimer_min[i],AVGLOOPCOUNT, MPI_DOUBLE, MPI_MIN, 0,computeComm); 
	}

	if(!computeRank)
	{
		// stream initialisations 
		char STREAM_kernels[4][100] = {"COPY", "SCALE", "ADD", "TRIAD"}; 
		// stream data size used in different kernels 
		double bytes[NUM_KERNELS]; 
		bytes[COPY]			= 2*ioParams->N*sizeof(double); 
		bytes[SCALE]		= 2*ioParams->N*sizeof(double); 
		bytes[ADD]			= 3*ioParams->N*sizeof(double); 
		bytes[TRIAD]		= 3*ioParams->N*sizeof(double); 


		// calculate max, min, avg across loops across ranks. 
		double minTime[NUM_KERNELS], maxTime[NUM_KERNELS]; 
		for(int i = 0; i < NUM_KERNELS; i++)
		{
			compTimer_avg[i] = 0.0; 
			maxTime[i] = compTimer_max[i][0]; 
			minTime[i] = compTimer_min[i][0]; 
			for(int j = 0; j < AVGLOOPCOUNT; j++)
			{
				compTimer_avg[i] += compTimer_max[i][j]; 
				if(maxTime[i] < compTimer_max[i][j])
				{
					maxTime[i] = compTimer_max[i][j];  
				} 
				if(minTime[i] > compTimer_min[i][j])
				{
					minTime[i] = compTimer_min[i][j];  
				} 
			}
			compTimer_avg[i] /= AVGLOOPCOUNT; 
		}
		
		int test = remove(FILENAME);
		out = fopen(FILENAME, "w+");
		if (out == NULL)
		{
			printf("Error: No output file\n");
			exit(1);
		}
		// header for compute output 
	  fprintf(out,"Function,Best_Rate(GB/s),Avg_time(s),Min_time(s),Max_time(s),Max_Walltime(s)\n");

    for (int i=0; i<NUM_KERNELS; i++) {
			fprintf(out,"%s,%lf,%lf,%lf,%lf,\n", STREAM_kernels[i],
	       1.0E-09 * bytes[i]/minTime[i],
	       compTimer_avg[i],
	       minTime[i],
	       maxTime[i]);
    }
		fprintf(out,",,,,,%lf \n", wallTime_max); 
	}
} 
