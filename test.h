#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>  

#define N 10 
#define error_check(ierr) if(ierr!=MPI_SUCCESS){ printf("mpi error \n"); exit(1);  }  

void initialise(int* array,int value); 
void printData(int* recv); 
// void dataSend(int len, MPI_Comm newComm, int* values); 
MPI_Win dataSend(int len, MPI_Comm newComm, int* values); 
void dataSendComplete(MPI_Comm newComm, MPI_Win win); 
void ioProcess(MPI_Comm newComm); 
MPI_Win createWindow(int len, MPI_Comm newComm, int* array); 
