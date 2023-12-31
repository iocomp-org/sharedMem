#include <omp.h> 
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <hdf5.h>
#include "sharedmem.h"

void phdf5write(double* iodata, char* FILENAME,  struct params* ioParams) 
{   
	// Variable initialisation
	int i,  rank, size; 
	int dims[NDIM],
			coords[NDIM], 
			periods[NDIM]; 

	char dsetname[100] = "IntArray"; 

	// HDF5 initialisations
	hid_t   file_id, // file identifier
					dset_id, // dataset identifier 
					filespace, // dataspace identifier in file 
					memspace, // dataspace identifier in memory
					plist_id;  // property list identifier 

	hsize_t count[NDIM], 
					offset[NDIM],
					dimsf[NDIM];   // specifies the dimensions of dataset, dimsf[0] number of rows, dimsf[1] number of columns, dimsf[2] so on..

	herr_t status = H5open();
	error_check(status); 

	
#ifndef NDEBUG 
	fprintf(ioParams->debug,"Before HDF5 open \n"); 
#endif 

	// Obtain MPI keys 
	MPI_Comm_size(ioParams->cartcomm, &size);
	MPI_Comm_rank(ioParams->cartcomm, &rank);
	MPI_Cart_get(ioParams->cartcomm, NDIM, dims, periods, coords); 
	MPI_Info info  = MPI_INFO_NULL; 
#ifndef NDEBUG 
	fprintf(ioParams->debug,"After MPI stuff \n"); 
#endif 

	/* 
	 * Initialisations from arrays passed in the function 
	 */ 
	for (i = 0; i < NDIM; i++)
	{
		dimsf[i]	= ioParams->globalArray[i]; 
		count[i]	= ioParams->localArray[i]; 
		offset[i] =	ioParams->arrayStart[i]; 
	}
#ifndef NDEBUG 
	fprintf(ioParams->debug,"Initialise dimsf, count, offset\n"); 
#endif 

	/* 
	 * Set up file access property list with parallel I/O access
	 */
	plist_id = H5Pcreate(H5P_FILE_ACCESS); 
	H5Pset_fapl_mpio(plist_id, ioParams->cartcomm, info);
#ifndef NDEBUG 
	fprintf(ioParams->debug,"File access property\n"); 
#endif 

	/*
	 * Create a new file collectively and release property list identifier.
	 */
	file_id = H5Fcreate(FILENAME, H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
	H5Pclose(plist_id);
#ifndef NDEBUG 
	fprintf(ioParams->debug,"New collective file object \n"); 
#endif 

	/*
	 * Create the dataspace for the dataset.
	 */
	filespace = H5Screate_simple(NDIM, dimsf, NULL); 
	dset_id = H5Dcreate(file_id, dsetname, H5T_NATIVE_DOUBLE, filespace, 
			H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); 
	H5Sclose(filespace);
#ifndef NDEBUG 
	fprintf(ioParams->debug,"Dataspace for dataset \n"); 
#endif 

	/* 
	 * Each process defines dataset in memory and writes it to the hyperslab
	 * in the file.
	 */
	memspace = H5Screate_simple(NDIM, count, NULL); 

	/*
	 * Select hyperslab in the file.
	 */
	filespace = H5Dget_space(dset_id); // makes a copy of the dataspace FOR the dataset dset_id
	H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL); 

	/*
	 * Create property list for collective dataset write.
	 */
	plist_id = H5Pcreate(H5P_DATASET_XFER); // sets data transfer mode.
	H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE); // sets data transfer mode.

	status = H5Dwrite (dset_id, H5T_NATIVE_DOUBLE, memspace, filespace, 
			plist_id, iodata);
#ifndef NDEBUG 
	fprintf(ioParams->debug,"After HDF5 write \n"); 
#endif 

	// Free resources
	status = H5Sclose (memspace);
	status = H5Sclose (filespace);
	status = H5Dclose (dset_id);    
	status = H5Pclose (plist_id); 
	status = H5Fclose (file_id);

}
