/**
 *  \author Miguel Maia 76434
 *  \tester Miguel Maia 76434
 */

/**
 *  \brief Write a data cluster.
 *
 *  Data is written into a specific data cluster which is supposed
 *  to belong to an inode associated to a file (a regular
 *  file, a directory or a symbolic link). 
 *
 *  If the referred cluster has not been allocated yet,
 *  it will be allocated now so that the data can be stored as its
 *  contents.
 *
 *  \param ih inode handler
 *  \param fcn file cluster number
 *  \param buf pointer to the buffer where data must be read into
 */

#include "filecluster.h"
#include "probing.h"
#include "exception.h"
#include <errno.h>
#include "core.h"
#include <dealers.h>

void soWriteFileCluster(int ih, uint32_t fcn, void *buf){
    soProbe(600, "soWriteFileCluster(%d, %u, %p)\n", ih, fcn, buf);    

    uint32_t cn;

    soGetFileCluster(ih,fcn,&cn);							// Get the cluster number of fcn

    if (cn == NULL_REFERENCE){								// if this happens, it means that the cluster number is invalid... the cluster doesn't exist, so we need to allocate one
    	soAllocFileCluster(ih,fcn,&cn);						
	}    

    soWriteCluster(cn, buf);								// writes the info in the cluster
}
