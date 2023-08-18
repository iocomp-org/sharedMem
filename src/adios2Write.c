#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <adios2_c.h>
#include <adios2/c/adios2_c_adios.h> 
#include <adios2/c/adios2_c_types.h>
#include "sharedmem.h"
#define CONFIG_FILE_ADIOS2 "config.xml"


void adioswrite(double* iodata, char* FILENAME, struct params *ioParams)
{   
	adios2_error errio; 
	adios2_adios *ierr; 

	printf("called adios2 \n"); 

	ioParams->ADIOS2_IOENGINES[0] = "HDF5"; 
	ioParams->ADIOS2_IOENGINES[1] = "BP4"; 
	ioParams->ADIOS2_IOENGINES[2] = "BP5";

	// ierr = adios2_init_mpi(ioParams->cartcomm); 
	ierr = adios2_init_config_mpi(CONFIG_FILE_ADIOS2, ioParams->cartcomm);		
	assert(ierr!=NULL); 
#ifndef NDEBUG
	fprintf(ioParams->debug, "adios2Write->config file read \n");
#endif
	
	printf("config file \n"); 
	// declare I/O engine, subtract 2 as ADIOS2 starts from ioLibNum=2 
	ioParams->io = adios2_declare_io(ioParams->adios, 
			ioParams->ADIOS2_IOENGINES[ioParams->ioLibNum-2]); //IO handler declaration
#ifndef NDEBUG
	fprintf(ioParams->debug, "adios2Write->adios2  declare I/O engine %s \n", ioParams->ADIOS2_IOENGINES[ioParams->ioLibNum-2]);
#endif

#ifndef NDEBUG
	printf("adios2Write->adios2Init flag %i \n", ioParams->adios2Init);
#endif
	if(!ioParams->adios2Init) // check if declared before so that adios2 variable is not defined again. 
	{
		ioParams->var_iodata = adios2_define_variable(ioParams->io, "iodata", adios2_type_double,NDIM,
				ioParams->globalArray, ioParams->arrayStart, ioParams->localArray, adios2_constant_dims_true); 
		ioParams->adios2Init = 1;  
#ifndef NDEBUG
		printf("adios2Write->variable defined \n");
#endif
	}

	adios2_engine *engine = adios2_open(ioParams->io, FILENAME, adios2_mode_write);
#ifndef NDEBUG
	printf("adios2Write->engine opened \n");
#endif

	adios2_step_status status; 
	errio = adios2_begin_step(engine, adios2_step_mode_update, 10.0, &status);   
#ifndef NDEBUG
	printf("adios2Write->begin step \n");
#endif

	errio = adios2_put(engine,ioParams->var_iodata, iodata, adios2_mode_deferred);
	error_check(errio); 
#ifndef NDEBUG
	printf("adios2Write->writing completed \n");
#endif

	errio = adios2_end_step(engine);
	error_check(errio); 
#ifndef NDEBUG
	printf("adios2Write->end step\n");
#endif	

	errio = adios2_flush(engine); 
	error_check(errio); 
#ifndef NDEBUG
	printf("adios2Write->flush I/O engine\n");
#endif	

	errio = adios2_close(engine);
	error_check(errio); 
#ifndef NDEBUG
	printf("adios2Write->engine closed \n");
#endif
}
