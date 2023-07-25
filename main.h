#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>  
#define NUM_WIN 3

#define N 10 
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
void mpiiowrite(double* iodata, int*arraysubsize, int* arraygsize, int* arraystart, int NDIM, MPI_Comm cartcomm, char* FILENAME); 
void mpiRead(char* FILENAME, MPI_Comm ioServerComm ); 
void phdf5write(double* iodata, int*arraysubsize, int* arraygsize, int* arraystart, int NDIM, MPI_Comm cartcomm, char* FILENAME); 
void fileWrite(double* iodata, int*arraysubsize, int* arraygsize, int* arraystart, int NDIM, MPI_Comm cartcomm, char* FILENAME, MPI_Comm ioComm); 
