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
#define AVGLOOPCOUNT 1

// define problem size and dimension of data 
#define N 10 
#define NDIM 1

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
void initialise(int* array,int value); 
void printData(int* recv); 
// void dataSend(int len, MPI_Comm newComm, int* values); 
MPI_Win dataSend(int len, MPI_Comm newComm, int* values); 
void dataSendComplete(MPI_Win win); 
void ioProcess(MPI_Comm newComm); 
MPI_Win createWindow(int len, MPI_Comm newComm, int* array); 
struct winElements winAlloc(int len, MPI_Comm newComm);  
// void ioServer(MPI_Comm newComm, MPI_Win win_ptr[NUM_WIN], int* array[NUM_WIN]); 
void ioServer(MPI_Comm ioComm, MPI_Comm newComm); 
void ioServerWrite(char* WRITEFILE, int* array, int elementsNum); 
void mpiiowrite(double* iodata, int*arraysubsize, int* arraygsize, int* arraystart, int ndim, MPI_Comm cartcomm, char* FILENAME); 
void mpiRead(char* FILENAME, MPI_Comm ioServerComm ); 
void phdf5write(double* iodata, int*arraysubsize, int* arraygsize, int* arraystart, int ndim, MPI_Comm cartcomm, char* FILENAME); 
void fileWrite(double* iodata, int*arraysubsize, int* arraygsize, int* arraystart, int ndim, MPI_Comm cartcomm, char* FILENAME, MPI_Comm ioComm); 
void compServer(MPI_Comm computeComm, MPI_Comm newComm, MPI_Comm globalComm); 
