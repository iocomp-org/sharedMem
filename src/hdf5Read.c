#include <omp.h> 
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <hdf5.h>
#include "sharedmem.h"
#define DATASETNAME "IntArray"

void phdf5Read(double *readData, char* fileName, struct params *ioParams) 
{   

	hid_t       file, dataset;         /* handles */
	hid_t       datatype, dataspace;
	hid_t       memspace;
	H5T_class_t class;                 /* datatype class */
	H5T_order_t order;                 /* data order */
	size_t      size;                  /*
																			* size of the data element
																			* stored in file
																			*/
	hsize_t     dimsm[NDIM];              /* memory space dimensions */
	herr_t      status;
	herr_t			ierr; 

	hsize_t      count[NDIM];              /* size of the hyperslab in the file */
	hsize_t      offset[NDIM];             /* hyperslab offset in the file */
	int          i, j, k, status_n, rank;

	hid_t plist_id, xfer_plist;		/* File access templates */

	 MPI_Info info = MPI_INFO_NULL;

	// specifies the dimensions of dataset, dimsf[0] number of rows, dimsf[1] number of columns, dimsf[2] so on..
	hsize_t localArray[NDIM], globalArray[NDIM], arrayStart[NDIM];   

	/* 
	 * Define hyperslab in the dataset. 
	 * NULL for contigous selection of dataset for stride 
	 * NULL for 1 element per dimension of dataset for block
	 */
	for (i = 0; i < NDIM; i++)
	{
		offset[i]		= (hsize_t)ioParams->arrayStart[i]; 
		count[i]		= (hsize_t)ioParams->localArray[i];  
		dimsm[i]		= (hsize_t)ioParams->globalArray[i];
	}


	/* 
	 * Open HDF5 file 
	 */
	// Set up file access property list with parallel I/O access
	plist_id = H5Pcreate(H5P_FILE_ACCESS); 
	// error_check(plist_id); 

	// Set up parallel access with communicator 	
	ierr = H5Pset_fapl_mpio(plist_id, ioParams->cartcomm, info);
	// error_check(ierr); 
#ifndef NDEBUG 
	fprintf(ioParams->debug,"File access property\n"); 
#endif 

	/*
	 * Open the file collectively 
	 */
	file = H5Fopen(fileName, H5F_ACC_RDONLY, plist_id);
	// error_check(file); 

	/* Release file-access template */
	ierr =H5Pclose(plist_id);
	// error_check(ierr); 

	/*
	 * Open datasets in the file object 
	 */
	dataset = H5Dopen(file, DATASETNAME, H5P_DEFAULT);
	// error_check(dataset); 

//	/*
//	 * Get datatype and dataspace handles and then query
//	 * dataset class, order, size, rank and dimensions.
//	 */
//	datatype  = H5Dget_type(dataset);     /* datatype handle */ 
//	class     = H5Tget_class(datatype);
//	order     = H5Tget_order(datatype);
//
//	size  = H5Tget_size(datatype);
//	printf(" Data size is %ld \n", size);
	
	// create a file dataspace independently 
	// stride and block are set to be NULL 
	dataspace = H5Dget_space(dataset);    /* dataspace handle */
	// error_check(dataspace); 
	ierr = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, 
			count, NULL);
	// error_check(ierr); 


	// create memory dataspace independently 
	memspace = H5Screate_simple(NDIM,count,NULL);   
	// error_check(memspace); 
	
	// set up collective transfer properties list 
	xfer_plist = H5Pcreate (H5P_DATASET_XFER);
	// error_check(xfer_plist); 
	ierr=H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);
	// error_check(ierr); 

	/* read data collectively */
	ierr = H5Dread(dataset, H5T_NATIVE_DOUBLE, memspace, dataspace,
		xfer_plist, readData);					    
	
	for(int i = 0; i < ioParams->localDataSize; i++)
	{
		printf("%lf, ", readData[i]); 
	} 
	// error_check(ierr); 

//	status = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, 
//			count, NULL);
//
//	/*
//	 * Define the memory dataspace.
//	 */
//	memspace = H5Screate_simple(NDIM,,NULL);   
//
//	/* 
//	 * Define memory hyperslab. 
//	 */
//	status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset, NULL, 
//			count, NULL);
//
//	/*
//	 * Read data from hyperslab in the file into the hyperslab in 
//	 * memory and display.
//	 */
//	status = H5Dread(dataset, H5T_NATIVE_DOUBLE, memspace, dataspace,
//			H5P_DEFAULT, readData);
//
//	for(int i = 0; i < ioParams->localDataSize; i++)
//	{
//		printf("%lf,", readData[i]); 
//	} 
//
	/*
	 * Close/release resources.
	 */
	H5Dclose(dataset);
	H5Sclose(dataspace);
	H5Sclose(memspace);
	H5Fclose(file);

}     
