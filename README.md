# sharedMem
Shared memory demonstrator. 
## Command line options 
N is problem size. 
io is I/O library selector, 0 for MPIIO, 1 for HDF5 
## Usage 
	../sharemem --N 10 --io 1 
## Preprocessor flags 
- NDEBUG : adding this flag will disable debug printouts 
- IOBW : adding this flag will enable bandwidth calculation and printing of
	output from ioServer 
- NODELETE : adding this flag will disable automatic deletion of output file 


