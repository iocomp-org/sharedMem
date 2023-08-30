#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <adios2_c.h>
#if ADIOS2_USE_MPI
#include <mpi.h>
#endif 
#include "sharedmem.h"

void adios2Read(double* iodata, char* FILENAME, struct params *ioParams)
{
		adios2_engine *engine = adios2_open(ioParams->io, FILENAME, adios2_mode_read);

    adios2_variable *var_greeting = adios2_inquire_variable(ioParams->io, "iodata");

    adios2_get(engine, var_greeting, iodata, adios2_mode_deferred);

    adios2_close(engine);
} 
