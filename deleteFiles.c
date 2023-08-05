#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h> 
#include "stream_post_ioserver.h"

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

void deleteFiles(struct params* iocompParams, int windowNum)
{
	int ierr; 
	ierr = nftw(iocompParams->WRITEFILE[windowNum], unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
	error_check(ierr); 
} 
