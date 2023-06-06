#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>  

#define N 10 
#define error_check(ierr) if(ierr!=MPI_SUCCESS){ printf("mpi error \n"); exit(1);  }  

void initialise(int* array,int value); 


void printData(int* recv); 


void computeProcess(int len, MPI_Comm newComm, int* array, int val); 

void ioProcess(MPI_Comm newComm, int* array); 
