#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include <string.h> 
#include <assert.h> 
#include <math.h>
#include "stream_post_ioserver.h"
#define FILENAME "ioserver_output.csv"

void verify(struct params *ioParams)
{
	double a , b, c, val; 
	
	// Initialise 
	
	a = 1.0;
	b = 2.0; 
	c = 0.0; 
	printf("Verification started \n"); 
	for(int iter = 0; iter < AVGLOOPCOUNT; iter++)
	{
		
		b = SCALAR * c; 

		c = a + b; 

		a = b + (SCALAR*c); 
#ifndef NDEBUG
		printf("a[%i] = %lf, b[%i] = %lf, c[%i] = %lf \n", iter, a, iter, b, iter, c); 
#endif 

		// read all the windows and iterations 
		for(int windowNum = 0; windowNum < NUM_WIN; windowNum++)
		{
			switch(windowNum)
			{
				case(0):
						val = a; 
						break; 
				case(1): 
						val = c; 
						break; 
				case(2): 
						val = b; 
						break; 
				default: 
						break; 
			} 
			mpiRead(ioParams, windowNum, iter, val); 
		} 
	} 
} 

