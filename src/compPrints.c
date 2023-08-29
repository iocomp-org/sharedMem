#include "sharedmem.h"
#define FILENAME "compserver_output.csv"

void compPrints(struct params *ioParams)
{
	FILE* out; 
	// stream initialisations 
	char STREAM_kernels[4][100] = {"COPY", "SCALE", "ADD", "TRIAD"}; 
	// stream data size used in different kernels 
	double bytes[NUM_KERNELS]; 
	bytes[COPY]			= 2*ioParams->localDataSize*sizeof(double); 
	bytes[SCALE]		= 2*ioParams->localDataSize*sizeof(double); 
	bytes[ADD]			= 3*ioParams->localDataSize*sizeof(double); 
	bytes[TRIAD]		= 3*ioParams->localDataSize*sizeof(double); 

	// calculate max, min, avg across loops across ranks. 
	double minTime[NUM_KERNELS], maxTime[NUM_KERNELS]; 
	for(int i = 0; i < NUM_KERNELS; i++)
	{
		ioParams->compTimer_avg[i] = 0.0; 
		maxTime[i] = ioParams->compTimer_max[i][0]; 
		minTime[i] = ioParams->compTimer_min[i][0]; 
		for(int j = 0; j < AVGLOOPCOUNT; j++)
		{
			ioParams->compTimer_avg[i] += ioParams->compTimer_max[i][j]; 
			if(maxTime[i] < ioParams->compTimer_max[i][j])
			{
				maxTime[i] = ioParams->compTimer_max[i][j];  
			} 
			if(minTime[i] > ioParams->compTimer_min[i][j])
			{
				minTime[i] = ioParams->compTimer_min[i][j];  
			} 
		}
		ioParams->compTimer_avg[i] /= AVGLOOPCOUNT; 
	}

	remove(FILENAME);
	out = fopen(FILENAME, "w+");
	if (out == NULL)
	{
		printf("Error: No output file\n");
		exit(1);
	}
	// header for compute output 
	fprintf(out,"Function,Best_Rate(GB/s),Avg_time(s),Min_time(s),Max_time(s),Max_Walltime(s)\n");

	for (int i=1; i<NUM_KERNELS; i++) {
		fprintf(out,"%s,%lf,%lf,%lf,%lf,\n", STREAM_kernels[i],
				1.0E-09 * bytes[i]/minTime[i],
				ioParams->compTimer_avg[i],
				minTime[i],
				maxTime[i]);
	}
	fprintf(out,"WALL,,,,,%lf \n", ioParams->wallTime_max); 
} 
