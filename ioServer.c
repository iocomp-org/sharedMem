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

void ioServer(MPI_Comm ioComm, MPI_Comm newComm)
{
	// allocate windows 
	double* array[NUM_WIN]; 
	MPI_Win win_ptr[NUM_WIN]; 
	int flag[NUM_WIN]; 	
	int ierr; 
	int soi = sizeof(double); 

	// timer variables 
	float timers_start[NUM_WIN];
	float timers_end[NUM_WIN];

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
					fileWrite(array[i], arraysubsize, arraygsize, arraystart, NDIM, cartcomm, WRITEFILE[i], ioComm); 
					timers_end[i] = MPI_Wtime(); // finish writing timer  
				}

				//	Post window for access to array 
				//	and start timer for that window 
				printf("ioServer Window %i post \n", i); 
				ierr = MPI_Win_post(group, 0, win_ptr[i]);
				error_check(ierr); 
#ifndef NDEBUG 
				printf("ioServer -> post MPI post for window %i \n", i); 
#endif 
				timers_start[i] = MPI_Wtime();

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
					fileWrite(array[i], arraysubsize, arraygsize, arraystart, NDIM, cartcomm, WRITEFILE[i], ioComm); 
					timers_end[i] = MPI_Wtime(); // finish writing timer  
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
		fileWrite(array[i], arraysubsize, arraygsize, arraystart, NDIM, cartcomm, WRITEFILE[i], ioComm); 
		timers_end[i] = MPI_Wtime(); // finish writing timer  
#ifndef NDEBUG 
		printf("MPI win free IO server reached\n"); 
#endif 

		// MPI_Barrier(MPI_COMM_WORLD); 	
		ierr = MPI_Win_free(&win_ptr[i]);
		error_check(ierr); 
		timers_end[i] = MPI_Wtime(); // finish writing timer  
#ifndef NDEBUG 
		printf("ioServer -> timer ended for array %i, time %lf \n", i, timers_end[i] - timers_start[i]); 
#endif 
	} 
} 
