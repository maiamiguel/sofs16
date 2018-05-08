/*
 * \author Daniel Alves
 * \tester Nelson Costa
 */

#include "mksofs.h"
#include "cluster.h"
#include "superblock.h"
#include "inode.h"
#include "exception.h"
#include "core.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


void fillInSuperBlock(SOSuperBlock * sbp, const char *name,
                      uint32_t ntotal, uint32_t itotal, uint32_t bpc)
{
    /*throw SOException(ENOSYS, __FUNCTION__);*/

    sbp->magic = 0xFFFF;			/* set magic number */
    sbp->version = VERSION_NUMBER;	/* set version number*/

    /* set name */
    unsigned int i = 0;
    for( ; name[i]!='\0' && i<PARTITION_NAME_SIZE; i++)
    {
      sbp->name[i] = name[i];
    }
    sbp->name[i] = '\0';

    sbp->mstat = PRU; 				/* flag signaling if the file system was properly unmounted - PRU - if properly unmounted*/

    sbp->csize = bpc;
    sbp->ntotal = ntotal;

    sbp->itstart = 1;				/* block 0 is superblock */
    sbp-> itsize = itotal / IPB;
    if(itotal%IPB != 0) sbp->itsize = sbp->itsize+1;
    sbp->itsize += (ntotal - (sbp->itsize +1))%sbp->csize;
    sbp->itotal = sbp->itsize * IPB;

    sbp->ifree = sbp->itotal -1;			/* first is root */
    sbp->ihead = 1;					/* 0 is root */
    sbp->itail = sbp->itotal - 1;		

    sbp->czstart = sbp->itsize + 1; /* cluster start point */
    sbp->ctotal = (ntotal - 1 - sbp->itsize) / bpc; /* number of total clusters*/

    uint32_t rpc = RPB * sbp->csize;
    sbp->crefs = 1 + ((sbp->ctotal - 2)/(rpc));
    sbp->cfree = sbp->ctotal - 1 - sbp->crefs;   /* number of free clusters */    

    /* Caches os references of free clusters */
    sbp->chead.cluster_number = 1;  /* first free cluster */
    sbp->chead.cluster_idx = 0;
    sbp->ctail.cluster_number = sbp->crefs;
    sbp->ctail.cluster_idx = (sbp->cfree) % (RPB * sbp->csize - 1);

    for (i = 0; i < FCT_CACHE_SIZE; i++)
    {
        sbp->chead.cache.ref[i] = NULL_REFERENCE;
        sbp->ctail.cache.ref[i] = NULL_REFERENCE;
    }

    sbp->chead.cache.in = 0; 
    sbp->chead.cache.out = 0; 

    sbp->ctail.cache.in = 0; 
    sbp->ctail.cache.out = 0; 
}
