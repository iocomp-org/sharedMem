/*
 *LOOP
 *rank 1: finish writing c
 *rank 0; c=a
 *rank 1: start writing c
 *rank 1: finish writing b
 *rank 0: b=scale(c)
 *rank 1; start writing b
 *rank 1: finish writing c
 *rank 0; c = a+b
 *rank 1: start writing c
 *rank 1: finish writing a
 *rank 0: a = b + scale*c
 *rank 1: start writing a
 *END LOOP
 */ 

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include <string.h> 
#include <assert.h> 
#include <math.h> 
#include "sharedmem.h"

int main(int argc, char** argv)
{
	//skipped initialization above
	MPI_Init(&argc, &argv);
	int ierr; 

	// MPI related initialisations 
	int globalRank, globalSize, colour; 
	MPI_Comm_rank(MPI_COMM_WORLD,&globalRank); 
	MPI_Comm_size(MPI_COMM_WORLD,&globalSize); 
	MPI_Comm newComm;
	
	// for splitting of communicators, MPI size needs to be greater than 1 
	assert(globalSize%2==0); 
	assert(globalSize>1); 
	
	// command line interactions to set the problem size and the IO library number  
	struct params ioParams; 
	initialise(argc, argv, &ioParams); 
	ioParams.globalRank = globalRank; 
	ioParams.globalSize = globalSize; 
	
	// brief hello world to check parameters 
	if(!globalRank)
	{
		double problemSize = ioParams.localDataSize*8/(pow(2,20)); // MiB 
		printf("Shared memory demonstrator with local problem size=%lfMiB, number of windows=%i, number of ranks=%i, I/O library=%i\n", 
		problemSize, NUM_WIN, globalSize, ioParams.ioLibNum); 
	} 

	/*
	 * Assuming IO process and Compute Process are mapped to physical and SMT cores
	 * if size = 10 then IO rank would be 5,6,..9
	 * and compute rank would be 0,1,..4
	 * Assign similar colours to corresponding I/O and compute Process
	 * i.e. rank 0 and rank 5 would have same colour and then same MPI
	 * Communicator 
	 */  
	// colour = globalRank%(globalSize/2); // IO rank and comp rank have same colour
	colour = (int)globalRank/2; // IO rank and comp rank have same colour
	ierr = MPI_Comm_split(MPI_COMM_WORLD, colour, globalRank, &newComm); 
	error_check(ierr); 

	int newRank; 
	ierr = MPI_Comm_rank(newComm,&newRank); 
	error_check(ierr); 

	// initialise DEBUG file per rank
#ifndef NDEBUG 
	initDebugFile(&ioParams, globalRank); 
	fprintf(ioParams.debug, "Global rank=%i, New rank=%i, Colour=%i \n", globalRank, newRank, colour); 
#endif 

	// input user given filenames for each window
	char filenames[NUM_WIN][100] = {"WinA", "WinC", "WinB"}; 
	fileNameInit(&ioParams, filenames); 

	// divide MPI comm world into compute or I/O server based on newRank which is
	// either 0 or 1 
	// assign compute and I/O rank 
	MPI_Comm computeComm, ioComm; 
	int computeRank, ioRank, computeSize, ioSize;

	if(newRank == 0)
	{
		colour = 0; 
		MPI_Comm_split(MPI_COMM_WORLD, colour, globalRank, &computeComm );
		MPI_Comm_rank(computeComm, &computeRank); 
		MPI_Comm_size(computeComm, &computeSize); 
		if(!computeRank)
		{
			printf("Compute ranks have size %i \n", computeSize); 
		}
	}
	else
	{
		colour = 1; 
		MPI_Comm_split(MPI_COMM_WORLD, colour, globalRank, &ioComm );
		MPI_Comm_rank(ioComm, &ioRank); 
		MPI_Comm_size(ioComm, &ioSize); 
		if(!ioRank)
		{
			printf("IO ranks have size %i \n", ioSize); 
		}
	}

	// initialise windows for each array in both Compute and I/O process
	if(newRank == 0)
	{
		compServer(computeComm, newComm, MPI_COMM_WORLD, &ioParams); 
	} 
	else 
	{
		ioServer(ioComm, newComm,&ioParams); 
	} 
		
	//close debug file object 
#ifndef NDEBUG 
	fprintf(ioParams.debug, "Before MPI finalised \n"); 
#endif 
#ifndef NDEBUG 
	fclose(ioParams.debug); 
#endif 

	MPI_Finalize();
	return 0;
} 

