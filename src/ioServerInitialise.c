#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <mpi.h>
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

	/*
	 * only when ioLib chosen is adios2 and one of its engines 
	 */ 
	if(ioParams->ioLibNum >=2 && ioParams->ioLibNum <= 4)
	{
		/*
		 * Initialise adios2 engines list  
		 */ 
		ioParams->ADIOS2_IOENGINES[0] = "HDF5"; 
		ioParams->ADIOS2_IOENGINES[1] = "BP4"; 
		ioParams->ADIOS2_IOENGINES[2] = "BP5";

		/*
		 * ADIOS2 initialise engine and IO object 
		 */ 
#if ADIOS2_USE_MPI
		ioParams->adios = adios2_init_config_mpi(CONFIG_FILE_ADIOS2, ioParams->cartcomm);  
#else 
		adios2_adios *adios = adios2_init();  
#endif 
		assert(ioParams->adios != NULL); 

#ifndef NDEBUG
		fprintf(ioParams->debug, "ADIOS2 initialised, adios2 engine is %s \n", ioParams->ADIOS2_IOENGINES[ioParams->ioLibNum-2] ); 
#endif

		ioParams->io = adios2_declare_io(ioParams->adios, 
				ioParams->ADIOS2_IOENGINES[ioParams->ioLibNum-2]); //IO handler declaration
		assert(ioParams->io != NULL); 
#ifndef NDEBUG
		fprintf(ioParams->debug, "ADIOS2 I/O declared \n "); 
#endif
	} 

	/*
	 * Initialise previous initialisation and element counter flag
	 */ 
	//	ioParams->previousInit = 0;  
	//	ioParams->previousCount = 0;  
	ioParams->adios2Init = 0;  
} 

