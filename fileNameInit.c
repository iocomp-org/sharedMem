#include <stdio.h>
#include <stdlib.h>  
#include <string.h> 
#include "stream_post_ioserver.h"
// each window can write to its own file, initialise write file name for
// each window number 

void fileNameInit(struct params* ioParams, char filenames[NUM_WIN][100])
{

		char EXT[10]; 

		// Get correct extension based on I/O library used 
		switch(ioParams->ioLibNum)
		{
			case 0:
				strcpy(EXT, ".dat"); 
				break; 
			case 1:
				strcpy(EXT, ".h5"); 
				break; 
			case 2:
				strcpy(EXT, ".h5"); 
				break; 
			case 3:
				strcpy(EXT, ".bp4"); 
				break; 
			case 4:
				strcpy(EXT, ".bp5"); 
				break; 
			default:
				printf("ioLibNum invalid, invalid extension applied \n"); 
				exit; 
		} 

		// assign correct filename in format <filename>_<iteration>.<extension>
		for(int i = 0; i < NUM_WIN; i++)
		{
			for(int j = 0; j < AVGLOOPCOUNT; j++)
			{
				char iter[5]; 
				sprintf(iter,  "%d", j);
				strcpy(ioParams->WRITEFILE[i][j], filenames[i]); 
				strcat(ioParams->WRITEFILE[i][j], "_"); 
				strcat(ioParams->WRITEFILE[i][j], iter); 
				strcat(ioParams->WRITEFILE[i][j], EXT); 
				// printf("write file for win %i and iteration %i = %s \n",i,j, output[i][j]); 
			} 
		} 
} 
