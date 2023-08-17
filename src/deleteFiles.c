#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h> 
#include "sharedmem.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);
    if (rv)
        perror(fpath);
    return rv;
}
#pragma GCC diagnostic pop 

void deleteFiles(struct params* ioParams)
{
	MPI_Barrier(ioParams->ioComm); 
	int ioRank;
	MPI_Comm_rank(ioParams->ioComm, &ioRank); 
	int ierr; 
	if(!ioRank)
	{
		for(int windowNum = 0; windowNum < NUM_WIN; windowNum++)
		{
			for(int loopCounter = 0; loopCounter < AVGLOOPCOUNT; loopCounter++)
			{
				ierr = nftw(ioParams->WRITEFILE[windowNum][loopCounter], unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
				error_check(ierr); 
			} 
		} 
	} 
} 
