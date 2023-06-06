CC=mpicc.mpich 
# CFLAGS=-I$(IDIR)

ODIR=obj
# LDIR =../lib

LIBS=-lm

DEPS=test.h 

MAIN=sharedmem 

SRCS = multiArray.c 

OBJS = $(SRCS:.c=.o)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(MAIN): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o sharedmem 
