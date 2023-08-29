#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <adios2_c.h>
#if ADIOS2_USE_MPI
#include <mpi.h>
#endif 
#include "sharedmem.h"

void adioswrite(double* iodata, char* FILENAME, struct params *ioParams)
{
	/* 
	 * Assert tests to check for negative values 
	 */ 
	for (int i = 0; i <NDIM; i++)
	{
		assert(ioParams->localArray[i]  > 0); 
		assert(ioParams->globalArray[i] > 0); 
	}
	adios2_error errio; 

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
} 
