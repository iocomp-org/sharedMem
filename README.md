artifact
---------

This repository contains various data, scripts, and utilities for the investigations of an I/O server implemented using MPI Shared Memory APIs. 
We welcome contributions!

## Organization ##
The repository is arranged as follows:

    <root>
        src/                     	# Artifact source code 
        LICENSE                  	# The software license governing the software in this repository
        plots/
		env.yml 	
        	data/                   # Collated benchmark data in csv files, to be fed to scripts
			iocomp/         # Data generated by iocomp using ARCHER2 stored in csv files within multiple directories 
			artifact/	# Data generated by artifact using ARCHER2 stored in csv files within multiple directories
			output/ 	# A placeholder directory for output from scripts
        	scripts/              	# Processing scripts
			datacollect.py	# Trawl directories for csv files, and export data in json format. 
			plots.py      	# Input data in json format and output plots in pdf format  
			main.py      	# Main function calling the plotting and analysis files  


# Set up of artifact
## Compilation 

	cd src 
 	mkdir Object_dirs
	make CC=<compiler> HDF5_DIR=<path to HDF5> ADIOS2_DIR=<path to ADIOS2>

## Run time
N is problem size per core.   
io is I/O library selector, 0 for MPIIO, 1 for HDF5, 2 for ADIOS2 HDF5, 3 for ADIOS2 BP4

	../sharemem --N 10 --io 1 
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
'artifactdir' is the directory containing output files from the artifact.
'iocompdir' is the directory containing output files from iocomp.
The plotting script will output the wall time plot and the computational time plot with the default directories for iocomp and artifact csv files. 

	python3 main.py --iocompdir=../data/iocomp/IOCOMP --artifactdir=../data/artifact/OUTPUT	

To save the plots the save option can be added, which would save the plots in the output directory.  

	python3 main.py --iocompdir=../data/iocomp/IOCOMP --artifactdir=../data/artifact/OUTPUT --save 	
