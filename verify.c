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
	int a, b, c; 
	a = STARTING_VAL; 
	int val; 
	for(int iter = 0; iter < AVGLOOPCOUNT; iter++)
	{
		for(int windowNum = 0; windowNum < NUM_WIN; windowNum++)
		{
			//  stream simulation
			switch(windowNum)
			{
				case(0):
					if(iter>0)
					{
						b = SCALAR * c; 
					} 
					b = SCALAR; 
					val = b; 
					break; 
				case(1):
					c = a + b; 
					val = c; 
					break; 
				case(2):
					a = b + SCALAR * c; 
					val = a; 
					break; 
			}

			// test against all windows 

			switch(ioParams->ioLibNum)
			{
				case (0): 
						mpiRead(ioParams, windowNum, iter, val); 
						break; 
				default: 
					break; 
			} 
		} 
	} 
} 

