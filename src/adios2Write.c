#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <adios2_c.h>
#if ADIOS2_USE_MPI
#include <mpi.h>
#endif 
#include "sharedmem.h"

#define CONFIG_FILE_ADIOS2 "config.xml"

void adioswrite(double* iodata, char* FILENAME, struct params *ioParams)
{

	/*
	 * Initialise adios2 engines list  
	 */ 
	ioParams->ADIOS2_IOENGINES[0] = "HDF5"; 
	ioParams->ADIOS2_IOENGINES[1] = "BP4"; 
	ioParams->ADIOS2_IOENGINES[2] = "BP5";

	// check cartcomm 
	int rank, size; 
	MPI_Comm_rank(ioParams->cartcomm, &rank);
	MPI_Comm_size(ioParams->cartcomm, &size);


	/*
	 * adios2 object declared before loop entered
	 * only when ioLib chosen is adios2 and one of its engines 
	 */ 
	if(ioParams->ioLibNum >=2 && ioParams->ioLibNum <= 4)
	{
#if ADIOS2_USE_MPI
    ioParams->adios = adios2_init_config_mpi(CONFIG_FILE_ADIOS2, ioParams->cartcomm);  
#else 
    adios2_adios *adios = adios2_init();  
#endif 
#ifndef NDEBUG
		fprintf(ioParams->debug, "config \n "); 
#endif
		assert(ioParams->adios != NULL); 
#ifndef NDEBUG
		fprintf(ioParams->debug, "filename is %s , adios2 engine is %s \n", FILENAME, ioParams->ADIOS2_IOENGINES[ioParams->ioLibNum-2] ); 
#endif
		ioParams->io = adios2_declare_io(ioParams->adios, 
				ioParams->ADIOS2_IOENGINES[ioParams->ioLibNum-2]); //IO handler declaration
		assert(ioParams->io != NULL); 
#ifndef NDEBUG
		fprintf(ioParams->debug, "adios declared \n "); 
#endif
	} 

	/* 
	 * Assert tests to check for negative values 
	 */ 
	for (int i = 0; i <NDIM; i++)
	{
		assert(ioParams->localArray[i]  > 0); 
		assert(ioParams->globalArray[i] > 0); 
	}
	adios2_error errio; 

	ioParams->adios2Init = 0; 

#ifndef NDEBUG
	fprintf(ioParams->debug, "adios2Write->adios2Init flag %i \n", ioParams->adios2Init);
#endif
	if(!ioParams->adios2Init) // check if declared before so that adios2 variable is not defined again. 
	{
#ifndef NDEBUG
		fprintf(ioParams->debug, "adios2 variable definition \n"); 
#endif
		ioParams->var_iodata = adios2_define_variable(ioParams->io, "iodata", adios2_type_double,NDIM,
				ioParams->globalArray, ioParams->arrayStart, ioParams->localArray, adios2_constant_dims_true); 
		ioParams->adios2Init = 1;  
#ifndef NDEBUG
		fprintf(ioParams->debug, "adios2Write->variable defined \n");
#endif
	}
			
	adios2_engine *engine = adios2_open(ioParams->io, FILENAME, adios2_mode_write);
#ifndef NDEBUG
	fprintf(ioParams->debug, "adios2Write->engine opened \n");
#endif

	adios2_step_status status; 
	errio = adios2_begin_step(engine, adios2_step_mode_update, 10.0, &status);   
#ifndef NDEBUG
	fprintf(ioParams->debug, "adios2Write->begin step \n");
#endif

	errio = adios2_put(engine,ioParams->var_iodata, iodata, adios2_mode_deferred);
	error_check(errio); 
#ifndef NDEBUG
	fprintf(ioParams->debug, "adios2Write->writing completed \n");
#endif

	errio = adios2_end_step(engine);
	error_check(errio); 
#ifndef NDEBUG
	fprintf(ioParams->debug, "adios2Write->end step\n");
#endif	

	errio = adios2_flush(engine); 
	error_check(errio); 
#ifndef NDEBUG
	fprintf(ioParams->debug, "adios2Write->flush I/O engine\n");
#endif	

	errio = adios2_close(engine);
	error_check(errio); 
#ifndef NDEBUG
	fprintf(ioParams->debug, "adios2Write->engine closed \n");
#endif

	// adios2_finalize(ioParams->adios);
} 
