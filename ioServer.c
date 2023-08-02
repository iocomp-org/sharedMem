/*
 * function gets shared array pointer using mpi win shared query 
 * and prints out data as proxy for writing to file 
 */

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include <string.h> 
#include <assert.h> 
#include "stream_post_ioserver.h"
#define FILENAME "ioserver_output.csv"

void ioServer(MPI_Comm ioComm, MPI_Comm newComm)
{
	// allocate windows 
	double* array[NUM_WIN]; 
	MPI_Win win_ptr[NUM_WIN]; 
	int flag[NUM_WIN]; 	
	int ierr; 
	int soi = sizeof(double); 

	// timer variables
	// winTime measures the time taken from issuing of the win start to win wait 
	// writeTime measures the time taken for file write to complete 
	double winTime[NUM_WIN][AVGLOOPCOUNT]; 
	double winTime_max[NUM_WIN][AVGLOOPCOUNT]; 
	double writeTime[NUM_WIN][AVGLOOPCOUNT]; 
	double writeTime_max[NUM_WIN][AVGLOOPCOUNT]; 
	double fileSize; 

	// declare array of write files 
	char WRITEFILE[NUM_WIN][10]; 

	// IO setup create cartesian communicators 	
	int ioRank, ioSize,  
			reorder = 0, 
			dims_mpi[NDIM] = { 0 },
			coords[NDIM] = { 0 },
			periods[NDIM] = { 0 };  

	MPI_Comm_rank(ioComm, &ioRank); 
	MPI_Comm_size(ioComm, &ioSize); 

	// Cartesian communicator setup 
	MPI_Comm cartcomm; 
	ierr = MPI_Dims_create(ioSize, NDIM, dims_mpi);
	error_check(ierr);
	ierr = MPI_Cart_create(ioComm, NDIM, dims_mpi, periods, reorder, &cartcomm); //comm
	error_check(ierr);
	ierr = MPI_Cart_coords(cartcomm, ioRank, NDIM, coords);
	error_check(ierr);

	// assign arrray size, subsize and global size 
	int arraysubsize[NDIM], arraygsize[NDIM], arraystart[NDIM]; 
	for(int i = 0; i < NDIM; i++)
	{
		arraysubsize[i] = N; 
		arraygsize[i] = N;
		arraystart[i] = 0; 
	}
	// first element of array start and size different 
	arraystart[0] = N*ioRank;
	arraygsize[0] = N*ioSize; 

	// allocate shared windows 
	for(int i = 0; i < NUM_WIN; i++)
	{
		ierr = MPI_Win_allocate_shared(0, soi, MPI_INFO_NULL, newComm, &array[i], &win_ptr[i]); 
		error_check(ierr);
#ifndef NDEBUG 
		printf("ioServer -> MPI allocated windows %i \n", i); 
#endif 
		// each window can write to its own file, initialise write file name for
		// each window number 
		int arrayNumInt = '0' + i; 
		char arrayNumChar = (char) arrayNumInt; 
		char arrayNumString[] = {arrayNumChar, '\0'}; 
		strcpy(WRITEFILE[i], arrayNumString); 
		char EXT[] = ".dat"; 
		// char EXT[] = "array.h5"; 
		strcat(WRITEFILE[i], EXT); 

		// delete the previous files 
		int test = remove(WRITEFILE[i]);
	} 

	// allocate arrays using window pointers 
	for(int i = 0; i < NUM_WIN; i++)
	{
		long int arraySize; 
		int dispUnit; 
		int ierr; 
		ierr = MPI_Win_shared_query(win_ptr[i], 0, &arraySize, &dispUnit, &array[i]); 
		error_check(ierr); 
#ifndef NDEBUG 
		printf("ioServer -> MPI shared query %i \n", i); 
#endif 
	} 

	// groups 
	MPI_Group comm_group, group;
	int ranks[2]; 
	for (int j=0; j<2; j++) {
		ranks[j] = j;   
	}
	MPI_Comm_group(newComm,&comm_group);

	/* Compute group consists of rank 0*/
	MPI_Group_incl(comm_group,1,ranks,&group); 

	// assign wintestflags int to test for messages from the compute server  
	int wintestflags[NUM_WIN]; 
	// declare mult variable to test for completion among all windows 
	int wintestmult = 1; 

#ifdef IOBW
	// loopCounter to assign timers per loop iteration for each window
	int loopCounter[NUM_WIN]; 
	for(int i = 0 ; i < NUM_WIN; i ++)
	{
		loopCounter[i] = 0; 
		// initialise timers 
		for(int j = 0; j < AVGLOOPCOUNT; j++)
		{
			winTime[i][j] = 0.0; 
			writeTime[i][j] = 0.0; 
		}
	}
#endif 
	// Test for window completion 
	do 
	{
		MPI_Bcast( wintestflags, NUM_WIN, MPI_INT, 0, newComm); 
#ifndef NDEBUG 
		printf("ioServer -> after MPI bcast, wintestflags [%i,%i,%i] \n", wintestflags[0], wintestflags[1], wintestflags[2]); 
#endif 

		// iterate across all windows 
		for(int i = 0; i < NUM_WIN; i++)
		{
			if(wintestflags[i] > WIN_DEACTIVATE) // anything over 0 means go for printing 
			{
				if(wintestflags[i]==WIN_WAIT) // in this case WIN WAIT is coming before WIN POST, but it assumes that WIN POST has been called before. 
				{
#ifndef NDEBUG 
					printf("ioServer -> flag negative and win wait implemented\n"); 
#endif 
					// wait for window completion 
					ierr = MPI_Win_wait(win_ptr[i]); 
					error_check(ierr); 
#ifdef IOBW
					writeTime[i][loopCounter[i]] = MPI_Wtime(); 
#endif 
					fileWrite(array[i], arraysubsize, arraygsize, arraystart, NDIM, cartcomm, WRITEFILE[i], ioComm); 
#ifdef IOBW
					// finish write time 
					writeTime[i][loopCounter[i]] = MPI_Wtime() - writeTime[i][loopCounter[i]]; 
					// finish winTime 
					winTime[i][loopCounter[i]] = MPI_Wtime() - winTime[i][loopCounter[i]];   
#endif 
				}

				//	Post window for starting access to array 
				ierr = MPI_Win_post(group, 0, win_ptr[i]);
				error_check(ierr); 
#ifndef NDEBUG 
				printf("ioServer -> post MPI post for window %i \n", i); 
#endif 
#ifdef IOBW	
				// start loopCounter after posting winPost 
				loopCounter[i]++; 
				winTime[i][loopCounter[i]] = MPI_Wtime();
#endif 

				// test for window completion 	
				ierr = MPI_Win_test(win_ptr[i], &flag[i]); 
				error_check(ierr);
#ifndef NDEBUG 
				printf("ioServer -> win test\n"); 
#endif 
				// if window is available to print then print and end timer 
				if(flag[i])
				{
#ifndef NDEBUG 
					printf("ioServer -> flag positive \n"); 
#endif
#ifdef IOBW
					writeTime[i][loopCounter[i]] = MPI_Wtime(); 
#endif 
					fileWrite(array[i], arraysubsize, arraygsize, arraystart, NDIM, cartcomm, WRITEFILE[i], ioComm); 
#ifdef IOBW
					// finish write time 
					writeTime[i][loopCounter[i]] = MPI_Wtime() - writeTime[i][loopCounter[i]]; 
					// finish winTime 
					winTime[i][loopCounter[i]] = MPI_Wtime() - winTime[i][loopCounter[i]];   
#endif 
				}
			} 
		}

		// check if no more messages left 
		wintestmult = 1;  // reset value 
		for(int j = 0; j < NUM_WIN; j++)
		{
			wintestmult *= wintestflags[j]; 
		} 
#ifndef NDEBUG 
		printf("ioServer -> wintestmult value %i\n", wintestmult); 
#endif 
	}while(!wintestmult);  // test for completion of all windows 

#ifndef NDEBUG 
	printf("ioServer -> loop server exited \n"); 
#endif 

	// free windows and pointer 
	// while freeing, check if there are any opened windows 
	for(int i = 0; i < NUM_WIN; i++)
	{
		// wait for completion of all windows 
		ierr = MPI_Win_wait(win_ptr[i]); 
		error_check(ierr); 
#ifdef IOBW
		writeTime[i][loopCounter[i]] = MPI_Wtime(); 
#endif 
		fileWrite(array[i], arraysubsize, arraygsize, arraystart, NDIM, cartcomm, WRITEFILE[i], ioComm); 
		// finish writing timer

#ifdef IOBW
		// finish write time 
		writeTime[i][loopCounter[i]] = MPI_Wtime() - writeTime[i][loopCounter[i]]; 
		// finish winTime 
		winTime[i][loopCounter[i]] = MPI_Wtime() - winTime[i][loopCounter[i]];   
#endif 
#ifndef NDEBUG 
		printf("MPI win free IO server reached\n"); 
#endif 
		ierr = MPI_Win_free(&win_ptr[i]);
		error_check(ierr); 
	} 

#ifdef IOBW
	// print out timers by reducing all the variables to get the maximum value 
	FILE* out;
	// initialise file object 
	if(!ioRank)
	{
		int test = remove(FILENAME);
		out = fopen(FILENAME, "w+");
		if (out == NULL)
		{
			printf("Error: No output file\n");
			exit(1);
		}
		// header for print statements
		fprintf(out,"Window Number, Window Time, Write Time, IO BW \n"); 
	} 

	// MPI reduction of writeTime array over all IO ranks 
	for(int i = 0; i < NUM_WIN; i++)
	{
		MPI_Reduce(writeTime[i], writeTime_max[i], AVGLOOPCOUNT, MPI_DOUBLE, MPI_MAX, 0, ioComm); 
		MPI_Reduce(winTime[i], winTime_max[i], AVGLOOPCOUNT, MPI_DOUBLE, MPI_MAX, 0, ioComm); 
	}

	// calculate file size for B/W calculation 
	fileSize = 1; 
	for(int i = 0; i < NDIM; i++)
	{
		fileSize *= arraysubsize[i]; 
	}
	
	// ioRank = 0 writes to file 
	if(!ioRank)
	{
		for(int j = 0; j < AVGLOOPCOUNT; j ++)
		{
			for(int i = 0; i < NUM_WIN; i++)
			{
				fprintf(out,"%i,%.3f, %.3f, %.3f \n",i,winTime_max[i][j],writeTime_max[i][j],fileSize/writeTime_max[i][j]); 
			} 
		} 
	}
#endif 
} 
