#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>  

// define number of shared memory windows 
#define NUM_WIN 3

// define STREAM computational kernels 
#define	NUM_KERNELS 4
#define COPY 0
#define SCALE 1
#define ADD 2 
#define TRIAD 3

// define avg loop count of stream kernels
#define AVGLOOPCOUNT 10

// define problem size and dimension of data 
#define NDIM 2

// define window control integers 
#define WIN_DEACTIVATE 0 
#define WIN_ACTIVATE 1 
#define WIN_WAIT 2
#define WIN_FREE -1

// define STREAM array windows 
#define WIN_A 0
#define WIN_C 1
#define WIN_B 2

// compute server definitions 
#define SCALAR 5 
#define STARTING_VAL 1

#define error_check(ierr) if(ierr!=MPI_SUCCESS){ printf("mpi error \n"); exit(1);  }  

struct winElements {
	MPI_Win win; 
	double* array; 
	}; 
/*
 * structure passes around to most functions in program 
 */ 
struct params 
{
	// for io_libraries function 
	// array description variables 
	size_t localArray[NDIM],	globalArray[NDIM], arrayStart[NDIM]; 
	// problem size in number of elements 
	int globalDataSize; 
	int localDataSize; 
	// select I/O library 
	int ioLibNum; 
	// filenames 
	char WRITEFILE[NUM_WIN][AVGLOOPCOUNT][100]; 

	// timer variables
	// winTime measures the time taken from issuing of the win start to win wait 
	// writeTime measures the time taken for file write to complete 
	double winTime_start[NUM_WIN]; 
	double winTime_end[NUM_WIN]; 
	double writeTime_start; 
	double writeTime_end; 
	double winTime[NUM_WIN][AVGLOOPCOUNT]; 
	double winTime_max[NUM_WIN][AVGLOOPCOUNT]; 
	double writeTime[NUM_WIN][AVGLOOPCOUNT]; 
	double writeTime_max[NUM_WIN][AVGLOOPCOUNT]; 
	double fileSize; 

	// communicators 
	MPI_Comm cartcomm, newComm, ioComm; 

}; 
extern struct iocomp_params iocompParams; 
void printData(int* recv); 
// void dataSend(int len, MPI_Comm newComm, int* values); 
MPI_Win dataSend(int len, MPI_Comm newComm, int* values); 
void dataSendComplete(MPI_Win win); 
void ioProcess(MPI_Comm newComm); 
MPI_Win createWindow(int len, MPI_Comm newComm, int* array); 
struct winElements winAlloc(int len, MPI_Comm newComm);  
// void ioServer(MPI_Comm newComm, MPI_Win win_ptr[NUM_WIN], int* array[NUM_WIN]); 
void ioServer(MPI_Comm ioComm, MPI_Comm newComm, struct params *ioParams); 
void ioServerWrite(char* WRITEFILE, int* array, int elementsNum); 
void mpiiowrite(double* iodata, int*arraysubsize, int* arraygsize, int* arraystart, int ndim, MPI_Comm cartcomm, char* FILENAME); 
void mpiRead(struct params *ioParams, int windowNum, int iter, double val); 
void phdf5write(double* iodata, int*arraysubsize, int* arraygsize, int* arraystart, int ndim, MPI_Comm cartcomm, char* FILENAME); 
void fileWrite(struct params *ioParams, double* iodata, int* loopCounter, int windowNum); 
void compServer(MPI_Comm computeComm, MPI_Comm newComm, MPI_Comm globalComm, struct params *ioParams); 
void initialise(int argc, char** argv, struct params *ioParams); 
void deleteFiles(struct params* iocompParams); 
void arrayParamsInit(struct params *iocompParams); 
void fileNameInit(struct params* ioParams, char filenames[NUM_WIN][100]); 
void verify(struct params *ioparams); 
int valueCheck(struct params *ioParams, double* iodata_test, double val, int windowNum, int iter); 
