CC=mpicc.mpich 
CFLAGS=-fPIC -g -Og #-DNDEBUG 

ODIR = Object_files

LFLAGS = # -I$(HDF5_DIR)/include

LIBS = #-L$(HDF5_DIR)/lib -lhdf5_hl -lhdf5 -lm 

DEPS=stream_post_ioserver.h

MAIN=sharedmem 

SRCS = main.c ioServer.c compServer.c mpiWrite.c mpiRead.c  fileWrite.c #hdf5Write.c

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
