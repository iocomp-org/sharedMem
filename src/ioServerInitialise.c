#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include "stdio.h"
#include "mpi.h"
#include "sharedmem.h"
#define config_file "config.xml"

// include ADIOS2 files if flag not defined 
#ifndef NOADIOS2 
#include <adios2_c.h>
#include <adios2/c/adios2_c_types.h>
#endif 

/*
 * Initialises the library 
 */
void ioServerInitialise(struct params *ioParams)
{
	/*
	 * MPI ranks and size 
	 */ 
	int ioSize, ioRank, ierr; 
	int reorder = 0; 
	ierr = MPI_Comm_size(ioParams->ioComm, &ioSize); 
	error_check(ierr); 
	ierr = MPI_Comm_rank(ioParams->ioComm, &ioRank); 
	error_check(ierr); 

	/*
	 * Initialisations for MPI cartesian communicator 
	 */ 
	int dims_mpi[NDIM]; 		
	int periods[NDIM]; 		
	int coords[NDIM]; 		

	/* 
	 * new communicator to which topology information is added 
	 */ 
	for(int i = 0; i < NDIM; i++)
	{
		dims_mpi[i] = 0; 
		coords[i] = 0; 
		periods[i] = 0; 
	}

	// Cartesian communicator setup 
	ierr = MPI_Dims_create(ioSize, NDIM, dims_mpi);
	error_check(ierr);
	ierr = MPI_Cart_create(ioParams->ioComm, NDIM, dims_mpi, periods, reorder, &ioParams->cartcomm); //comm
	error_check(ierr);
	ierr = MPI_Cart_coords(ioParams->cartcomm, ioRank, NDIM, coords);
	error_check(ierr);

	///*	
	// * Initiliase filename 
	// */ 
	//ioParams->FILENAMES[0] = "mpiio.dat"; 
	//ioParams->FILENAMES[1] = "hdf5.h5"; 
	//ioParams->FILENAMES[2] = "adios2.h5";
	//ioParams->FILENAMES[3] = "adios2.bp4";
	//ioParams->FILENAMES[4] = "adios2.bp5"; 
	///*
	// * Initialise adios2 engines list  
	// */ 
	//ioParams->ADIOS2_IOENGINES[0] = "HDF5"; 
	//ioParams->ADIOS2_IOENGINES[1] = "BP4"; 
	//ioParams->ADIOS2_IOENGINES[2] = "BP5";

	/*
	 * adios2 object declared before loop entered
	 * only when ioLib chosen is adios2 and one of its engines 
	 */ 
	if(ioParams->ioLibNum >=2 && ioParams->ioLibNum <= 4)
	{
		ioParams->adios = adios2_init_config_mpi(config_file, ioParams->cartcomm); 
		ioParams->io = adios2_declare_io(ioParams->adios, 
				ioParams->ADIOS2_IOENGINES[ioParams->ioLibNum-2]); //IO handler declaration
	} 

	/*
	 * Initialise previous initialisation and element counter flag
	 */ 
//	ioParams->previousInit = 0;  
//	ioParams->previousCount = 0;  
//	ioParams->adios2Init = 0;  
} 

