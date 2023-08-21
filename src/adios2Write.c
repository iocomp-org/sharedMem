#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <adios2_c.h>
#if ADIOS2_USE_MPI
#include <mpi.h>
#endif 
#include "sharedmem.h"

#define CONFIG_FILE_ADIOS2 "config.xml"

//void adioswrite(double* iodata, char* FILENAME, struct params *iocompParams)
//{
//
//	/*
//	 * Initialise adios2 engines list  
//	 */ 
//	iocompParams->ADIOS2_IOENGINES[0] = "HDF5"; 
//	iocompParams->ADIOS2_IOENGINES[1] = "BP4"; 
//	iocompParams->ADIOS2_IOENGINES[2] = "BP5";
//
//	// check cartcomm 
//	int rank, size; 
//	MPI_Comm_rank(iocompParams->cartcomm, &rank);
//	MPI_Comm_size(iocompParams->cartcomm, &size);
//
//
//	/*
//	 * adios2 object declared before loop entered
//	 * only when ioLib chosen is adios2 and one of its engines 
//	 */ 
//	if(iocompParams->ioLibNum >=2 && iocompParams->ioLibNum <= 4)
//	{
//		// iocompParams->adios = adios2_init_config_mpi(CONFIG_FILE_ADIOS2, iocompParams->cartcomm); 
//		iocompParams->adios = adios2_init_mpi(iocompParams->cartcomm); 
//		printf("config \n "); 
//		assert(iocompParams->adios != NULL); 
//		printf("filename is %s , adios2 engine is %s \n", FILENAME, iocompParams->ADIOS2_IOENGINES[iocompParams->ioLibNum-2] ); 
//		iocompParams->io = adios2_declare_io(iocompParams->adios, 
//				iocompParams->ADIOS2_IOENGINES[iocompParams->ioLibNum-2]); //IO handler declaration
//		assert(iocompParams->io != NULL); 
//		printf("adios declared \n "); 
//	} 
//
//	/* 
//	 * Assert tests to check for negative values 
//	 */ 
//	for (int i = 0; i <NDIM; i++)
//	{
//		assert(iocompParams->localArray[i]  > 0); 
//		assert(iocompParams->globalArray[i] > 0); 
//	}
//	adios2_error errio; 
//
//	iocompParams->adios2Init = 0; 
//
//#ifndef NDEBUG
//	printf("adios2Write->adios2Init flag %i \n", iocompParams->adios2Init);
//#endif
//	if(!iocompParams->adios2Init) // check if declared before so that adios2 variable is not defined again. 
//	{
//		printf("adios2 variable definition \n"); 
//		iocompParams->var_iodata = adios2_define_variable(iocompParams->io, "iodata", adios2_type_double,NDIM,
//				iocompParams->globalArray, iocompParams->arrayStart, iocompParams->localArray, adios2_constant_dims_true); 
//		iocompParams->adios2Init = 1;  
//#ifndef NDEBUG
//		printf("adios2Write->variable defined \n");
//#endif
//	}
//			
//	adios2_engine *engine = adios2_open(iocompParams->io, FILENAME, adios2_mode_write);
//#ifndef NDEBUG
//	printf("adios2Write->engine opened \n");
//#endif
//
//	adios2_step_status status; 
//	errio = adios2_begin_step(engine, adios2_step_mode_update, 10.0, &status);   
//#ifndef NDEBUG
//	printf("adios2Write->begin step \n");
//#endif
//
//	errio = adios2_put(engine,iocompParams->var_iodata, iodata, adios2_mode_deferred);
//	error_check(errio); 
//#ifndef NDEBUG
//	printf("adios2Write->writing completed \n");
//#endif
//
//	errio = adios2_end_step(engine);
//	error_check(errio); 
//#ifndef NDEBUG
//	printf("adios2Write->end step\n");
//#endif	
//
//	errio = adios2_flush(engine); 
//	error_check(errio); 
//#ifndef NDEBUG
//	printf("adios2Write->flush I/O engine\n");
//#endif	
//
//	errio = adios2_close(engine);
//	error_check(errio); 
//#ifndef NDEBUG
//	printf("adios2Write->engine closed \n");
//#endif
//
//	adios2_finalize(iocompParams->adios);
//} 





void adioswrite(double* iodata, char* FILENAME, struct params *ioParams)
{
		adios2_error errio;
		ioParams->ADIOS2_IOENGINES[0] = "HDF5"; 
		ioParams->ADIOS2_IOENGINES[1] = "BP4"; 
		ioParams->ADIOS2_IOENGINES[2] = "BP5";
    /* 
    * Initialisations 
    */ 
		printf("adios2 start \n"); 
#if ADIOS2_USE_MPI
    // adios2_adios *adios = adios2_init_config_mpi(CONFIG_FILE_ADIOS2, ioParams->cartcomm);  
    adios2_adios *adios = adios2_init_mpi(ioParams->cartcomm);  
#else 
    adios2_adios *adios = adios2_init();  
#endif 
		assert(adios != NULL); 
		printf("adios2 config \n"); 

		// adios2_io *io = adios2_declare_io(adios, "BP4"); //IO handler declaration
		adios2_io *io = adios2_declare_io(adios, 
			ioParams->ADIOS2_IOENGINES[ioParams->ioLibNum-2]); //IO handler declaration
		assert(io != NULL); 
		printf("adios2 declare io\n"); 
    
    /*
    * constant_dims true variables constant, false variables can change after definition. For every rank this should be true.
    * shape is global dimension
    * start is local offset 
    * count is local dimension
    */

		adios2_variable *var_iodata = adios2_define_variable(io, "iodata", adios2_type_double,NDIM,
				ioParams->globalArray, ioParams->arrayStart, ioParams->localArray, adios2_constant_dims_true); 
		printf("adios2 variable \n");

		printf("filename %s \n", FILENAME); 

		adios2_engine *engine = adios2_open(io, FILENAME, adios2_mode_write);
		printf("adios2 engine \n"); 

    adios2_step_status status; 

    errio = adios2_begin_step(engine, adios2_step_mode_update, 10.0, &status);   
		printf("adios2 step start\n"); 
		error_check(errio);

    errio = adios2_put(engine, var_iodata, iodata, adios2_mode_deferred);
		printf("adios2 put \n"); 
		error_check(errio);

    errio = adios2_end_step(engine);
		printf("adios2 end step \n"); 
		error_check(errio);

		errio = adios2_flush(engine);
		error_check(errio);

    errio = adios2_close(engine);
		error_check(errio);

    errio = adios2_finalize(adios);
		printf("adios2 finalize\n"); 
		error_check(errio);
} 
