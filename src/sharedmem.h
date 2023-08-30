#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>  
#include <adios2_c.h>
#include <adios2/c/adios2_c_types.h>

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
#define NDIM 2

// define window control integers 
#define WIN_DEACTIVATE 0 
#define WIN_TEST 1
#define WIN_ACTIVATE 2 
#define WIN_WAIT 3
#define WIN_FREE -1

// define STREAM array windows 
#define WIN_A 0
#define WIN_C 1
#define WIN_B 2

// compute server definitions 
#define SCALAR 5 
#define STARTING_VAL 1

// adios2 config file 
#define CONFIG_FILE_ADIOS2 "config.xml"

/*
 * Header file for declaring the error_report_fn and macro error_report which simplifies calling.
 */
#ifndef _ERROR_REPORT_H_
#define _ERROR_REPORT_H_

/*
 * error_report macro.
 * Takes an error code as input, expanding to call the error_report_fn with the line number and file
 * name values defined by the __LINE___ and __FILE__ macros.
 */
#define error_check(ierr) error_report_fn(ierr, __LINE__, __FILE__)
void error_report_fn(int ierr, int line_no, char *file_name);
#endif

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
	// shared window pointers 
	MPI_Win win_ptr[NUM_WIN]; 

	// initialise flag variable to test for window completion
	int flagReturn[NUM_WIN];

	// flag to test if write has been completed 
	int writeComplete[NUM_WIN]; 

	// file object for debug 
#ifndef NDEBUG
	char debugFile[100]; 
	FILE* debug; 
#endif 

	// rank and size 
	int globalRank, globalSize; 
	int newRank, newSize; 

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

	double compTimer[NUM_KERNELS][AVGLOOPCOUNT]; 
	double compTimer_max[NUM_KERNELS][AVGLOOPCOUNT]; 
	double compTimer_min[NUM_KERNELS][AVGLOOPCOUNT]; 
	double compTimer_avg[NUM_KERNELS]; 
	double wallTime_max; 

	// communicators 
	MPI_Comm cartcomm, newComm, ioComm; 

	// ADIOS2 
	char *ADIOS2_IOENGINES[3];
	int adios2Init; 
	// adios2 object 
	adios2_adios *adios;  
	// adios2 io object 
	adios2_io* io; 
	// adios2 variable object 
	adios2_variable *var_iodata; 


}; 
extern struct iocomp_params iocompParams; 
void printData(int* recv); 
// void dataSend(int len, MPI_Comm newComm, int* values); 
MPI_Win dataSend(int len, MPI_Comm newComm, int* values); 
void dataSendComplete(MPI_Win win, struct params *ioParams); 
void ioProcess(MPI_Comm newComm); 
MPI_Win createWindow(int len, MPI_Comm newComm, int* array); 
struct winElements winAlloc(int len, MPI_Comm newcomm, struct params *ioparams); 
// void ioServer(MPI_Comm newComm, MPI_Win win_ptr[NUM_WIN], int* array[NUM_WIN]); 
void ioServer(MPI_Comm ioComm, MPI_Comm newComm, struct params *ioParams); 
void ioServerWrite(char* WRITEFILE, int* array, int elementsNum); 
void fileWrite(struct params *ioParams, double* iodata, int* loopCounter, int windowNum); 
void compServer(MPI_Comm computeComm, MPI_Comm newComm, MPI_Comm globalComm, struct params *ioParams); 
void initialise(int argc, char** argv, struct params *ioParams); 
void deleteFiles(struct params* iocompParams); 
void arrayParamsInit(struct params *iocompParams); 
void fileNameInit(struct params* ioParams, char filenames[NUM_WIN][100]); 
void verify(struct params *ioparams); 
int valueCheck(struct params *ioParams, double* iodata_test, double val, int windowNum, int iter); 
void initDebugFile(struct params* ioParams, int globalRank); 
void ioServerInitialise(struct params *ioParams); 
void compPrints(struct params *ioParams); 
void winTest(struct params *ioParams,double* array, int windowNum, int* loopCounter); 

// Writing functions 
void mpiiowrite(double* iodata, char* FILENAME, struct params* ioParams);
void adioswrite(double* iodata, char* FILENAME, struct params *ioParams); 
void phdf5write(double* iodata, char* FILENAME, struct params* ioParams); 

// Reading functions 
void mpiRead(double *readData, char* FILENAME,  struct params *iocompParams); 
void phdf5Read(double *readData, char* FILENAME, struct params *iocompParams); 
void adios2Read(double* readData, char* FILENAME, struct params *iocompParams); 
