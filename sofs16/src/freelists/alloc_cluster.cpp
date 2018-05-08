/**
 *  \author Vitor Morais
 *  \tester Vitor Morais
 */

#include "freelists.h"
#include <core.h>
#include "probing.h"
#include "exception.h"
#include "superblock.h"
#include "sbdealer.h"
#include <errno.h>

/*
 * Dictates to be obeyed by the implementation:
 * - error ENOSPC should be thrown if there is no free clusters
 * - after the reference is removed, 
 *      its location should be filled with NULL_REFERENCE
 */
void soAllocCluster(uint32_t * cnp)
{
    soProbe(713, "soAllocCluster(%u)\n", cnp);

    SOSuperBlock *sbp;				
    sbp = sbGetPointer();

    if(sbp->cfree == 0){
    	throw SOException(ENOSPC, __FUNCTION__);
    }

   	if(sbp->chead.cache.out - sbp->chead.cache.in == 0){
   		soReplenish();
   	}

   	*cnp = sbp->chead.cache.ref[sbp->chead.cache.out];

   	sbp->chead.cache.ref[sbp->chead.cache.out] = NULL_REFERENCE;

	  sbp->chead.cache.out=(sbp->chead.cache.out+1)%FCT_CACHE_SIZE;
    
    sbp->cfree--;
   	
   	sbSave();
}

