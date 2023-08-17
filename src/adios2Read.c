#include <stdlib.h>
#include <string.h>
#include <adios2_c.h>
#include <adios2/c/adios2_c_adios.h> 
#include <adios2/c/adios2_c_types.h>
#include "sharedmem.h"
#define config_file "config.xml"


void adios2Read(double* readData, char* FILENAME, struct params *iocompParams)
{   
	adios2_error errio; 

	iocompParams->ADIOS2_IOENGINES[0] = "HDF5"; 
	iocompParams->ADIOS2_IOENGINES[1] = "BP4"; 
	iocompParams->ADIOS2_IOENGINES[2] = "BP5";

	// adios2_adios *adios = adios2_init_config_mpi(config_file, iocompParams->cartcomm); 
	adios2_init_config_mpi(config_file, iocompParams->cartcomm); 

	// iocompParams->adios = adios2_init_config_mpi(config_file, iocompParams->cartcomm); 
	iocompParams->io = adios2_declare_io(iocompParams->adios, 
			iocompParams->ADIOS2_IOENGINES[iocompParams->ioLibNum]); //IO handler declaration

#ifndef NDEBUG
	printf("adios2Write->adios2Init flag %i \n", iocompParams->adios2Init);
#endif
	if(!iocompParams->adios2Init) // check if declared before so that adios2 variable is not defined again. 
	{
		iocompParams->var_iodata = adios2_define_variable(iocompParams->io, "iodata", adios2_type_double,NDIM,
				iocompParams->globalArray, iocompParams->arrayStart, iocompParams->localArray, adios2_constant_dims_true); 
		iocompParams->adios2Init = 1;  
#ifndef NDEBUG
		printf("adios2Write->variable defined \n");
#endif
	}

	adios2_engine *engine = adios2_open(iocompParams->io, FILENAME, adios2_mode_write);
#ifndef NDEBUG
	printf("adios2Write->engine opened \n");
#endif

	adios2_step_status status; 
	errio = adios2_begin_step(engine, adios2_step_mode_update, 10.0, &status);   
#ifndef NDEBUG
	printf("adios2Write->begin step \n");
#endif

	errio = adios2_get(engine,iocompParams->var_iodata, readData, adios2_mode_deferred);
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
