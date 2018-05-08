/**
 *  \author Miguel Maia 76434
 *  \tester Miguel Maia 76434
 */

/**
 *  \brief Read a file cluster.
 *
 *  Data is read from a specific data cluster which is supposed to belong 
 *  to an inode associated to a file (a regular
 *  file, a directory or a symbolic link). 
 *
 *  If the referred file cluster has not been allocated yet, 
 *  the returned data will consist of a byte stream filled with the
 *  character null (ascii code 0).
 *
 *  \param ih inode handler
 *  \param fcn file cluster number
 *  \param buf pointer to the buffer where data must be read into
 */

#include "filecluster.h"
#include "probing.h"
#include "exception.h"
#include <errno.h>
#include <dealers.h>
#include "core.h"

void soReadFileCluster(int ih, uint32_t fcn, void *buf){
    soProbe(600, "soReadFileCluster(%d, %u, %p)\n", ih, fcn, buf);

    uint32_t cn;

    soGetFileCluster(ih,fcn,&cn);							// Get the cluster number of fcn

    if (cn == NULL_REFERENCE){								// if this happens, it means that the cluster number is invalid, so it needs to be set with '\0'
    	memset(buf,'\0',soGetBPC());
	}
	else{
		soReadCluster(cn,buf);								// Reads the info to the buffer
	}
}
