CC=cc 
CFLAGS= -fPIC -O2 -Wall -Wpedantic -Werror -Wextra -DADIOS2_USE_MPI 
CFLAGS+= -DVERIFY # -DNDEBUG  #-DNDEBUG #-DNODELETE -DIOBW -DVERIFY 

ODIR = Object_files

LFLAGS = -I$(HDF5_DIR)/include -I$(ADIOS2_DIR)/include

LIBS = -L$(HDF5_DIR)/lib -lhdf5_hl -lhdf5 -lm -L$(ADIOS2_DIR)/lib -ladios2_c -ladios2_c_mpi -L$(ADIOS2_DIR)/lib64 -ladios2_c -ladios2_c_mpi 

DEPS=sharedmem.h

MAIN=sharedmem 

SRCS = main.c ioServer.c compServer.c initialise.c  mpiWrite.c arrayParamsInit.c verify.c  fileNameInit.c fileWrite.c hdf5Write.c deleteFiles.c  mpiRead.c valueCheck.c checks.c initDebugFile.c winAlloc.c hdf5Read.c  adios2Write.c adios2Read.c ioServerInitialise.c iobw.c compPrints.c winTest.c 

_OBJS = $(SRCS:.c=.o)
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) ${LFLAGS} -c $< -o $@

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) ${LFLAGS} -o $(MAIN) $(OBJS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(CFLAGS) $^

# DO NOT DELETE THIS LINE -- make depend needs it
