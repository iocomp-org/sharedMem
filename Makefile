CC=mpicc.mpich 
CFLAGS=-DNDEBUG 

ODIR=obj
# LDIR =../lib

LIBS=-lm

DEPS=stream_post_ioserver.h

MAIN=sharedmem 

SRCS = stream_post_ioserver.c

OBJS = $(SRCS:.c=.o)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(MAIN): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o sharedmem 
