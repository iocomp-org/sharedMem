sharedMem
---------

This repository contains various data, scripts, and utilities for the investigations of an I/O server implemented using MPI Shared Memory APIs. 
We welcome contributions!

## Organization ##
The repository is arranged as follows:

    <root>
        src/                     	# sharedMem source code 
        LICENSE                  	# The software license governing the software in this repository
        plots/
		env.yml 	
        	data/                   # Collated benchmark data in csv files, to be fed to scripts
			iocomp/         # Data generated by iocomp using ARCHER2 stored in csv files within multiple directories 
			sharedMem/	# Data generated by sharedMem using ARCHER2 stored in csv files within multiple directories
			output/ 	# A placeholder directory for output from scripts
        	scripts/              	# Processing scripts
			datacollect.py	# Trawl directories for csv files, and export data in json format. 
			plots.py      	# Input data in json format and output plots in pdf format  
			main.py      	# Main function calling the plotting and analysis files  


# Set up of sharedMem

## Dependencies 
The repository has been tested with MPICH-4.0, ADIOS2-2.9.1, HDF5-1.12.1 although it runs with earlier versions of ADIOS2 and HDF5 as well. 
The instructions for installing HDF5 at the time of development are: 

	wget  "https://support.hdfgroup.org/ftp/HDF5
	/releases/hdf5-1.12/hdf5-1.12.2/
	src/hdf5-1.12.2.tar.gz"
	tar -xvf hdf5-1.12.2.tar.gz 
	cd hdf5-1.12.0
	CC=<path to compiler> ./configure  --enable-parallel --prefix=<install-directory>
	make && make check && make install 
 
The instructions for installing ADIOS2 at the time of development are: 

	git clone https://github.com/ornladios/ADIOS2.git
	mkdir adios2-build && cd adios2-build
	cmake -DHDF5_ROOT = <HDF5 LOCATION>  -DMPI_C_COMPILER=<MPI COMPILER>  -DADIOS2_USE_HDF5  -DCMAKE_INSTALL_PREFIX=<INSTALL_LOCATION> ../ADIOS2 
	make && make install 
 
## Compilation 

	cd src 
 	mkdir Object_dirs
	make CC=<compiler> HDF5_DIR=<path to HDF5> ADIOS2_DIR=<path to ADIOS2>

## Run time

Command line arguments
- N : Local problem size per core  
- io : I/O backend selection. 
    - 0 : MPIIO
    - 1 : HDF5 
    - 2 : ADIOS2_HDF5 
    - 3 : ADIOS2_BP4 

The executable only runs with an even number of processes and will exit if this requirement is not met. 

Sample runtime command for a local system is: 

	cd run_dir
	mpirun.mpich ../sharemem --N 10 --io 1 
 
### Preprocessor flags 
- NDEBUG : adding this flag will disable debug printouts 
- IOBW : adding this flag will enable bandwidth calculation and printing of
	output from ioServer 
- NODELETE : adding this flag will disable automatic deletion of output file 

# Run jobs on ARCHER2 
runscript.sh contains the following variables:
- IOSTART : starting I/O library
- IOEND : ending I/O library  
- ARRAY : SLURM job array options for averaging 
- LOCAL SIZE : local problem size 
- DIR : parent directory to store output files and sub directories
- QOS : standard or lowpriority determines the quality of service on ARCHER2
- TIMES : array of times per job 
- NODE_START : starting value for loop over number of nodes as power of 2s. 
- NODE_END : ending value for loop over number of nodes as power of 2s. 

## run scripts for ARCHER2 
	cd run_dir
	bash runscript.sh 


# Plotting setup and running 
## Conda environment 
Use the environment yml file to setup the environment for the plot scripts using conda.

	conda env create --file env.yml 

## Run plot scripts 
Extract data used in research paper and run the plot script.
'sharedMemdir' is the directory containing output files from the sharedMem.
'iocompdir' is the directory containing output files from iocomp.
The plotting script will output the wall time plot and the computational time plot with the default directories for iocomp and sharedMem csv files. 

	python3 main.py --iocompdir=../data/iocomp/IOCOMP --sharedMemdir=../data/sharedMem/OUTPUT	

To save the plots the save option can be added, which would save the plots in the output directory.  

	python3 main.py --iocompdir=../data/iocomp/IOCOMP --sharedMemdir=../data/sharedMem/OUTPUT --save 	
