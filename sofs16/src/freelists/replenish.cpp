/**
 *  \author Armando Sousa 76498
 *  \tester Nelson Costa 42983, Daniel Alves 76469 */

#include "freelists.h"
#include "probing.h"
#include "exception.h"
#include <errno.h>
#include <core.h>
#include "superblock.h"
#include "sbdealer.h"
#include "czdealer.h"

/*
 * Even if some of them are not functionally necessary,
 * the following dictates must be obeyed by the implementation:
 * - do nothing if head cache is not empty;
 * - the head cache should be filled from the beginning, 
 *      so, at the end, index out should be put at zero;
 * - if crefs is equal to zero, transfer references from the tail cache;
 * - otherwise, transfer references only from the head cluster;
 * - if after the transfer the head cluster get empty, it should be freed;
 * - after every reference is transferred, 
 *      the previous location should be filled with NULL_REFERENCE
 */
void soReplenish(void)
{

    soProbe(733, "soReplenish()\n");
    //throw SOException(ENOSYS, __FUNCTION__);

    SOSuperBlock* sbp = sbGetPointer();

    if (sbp->chead.cache.in == sbp->chead.cache.out)
    {
        if (sbp->crefs == 0) 	
            soDeplete();
        else{
            uint32_t rpc = soGetRPC();  /* Variável guarda a informação do número de referências por cluster (valor por defeito: 256) */
            uint32_t buf[rpc];          /* Funciona como buffer e recebe o conteúdo de um cluster */
            uint32_t * ref;
            
            soReadCluster(sbp->chead.cluster_number, buf);
            
            ref = (uint32_t *) buf;

            for (uint32_t i = 0; i < FCT_CACHE_SIZE; i++)
            {
                sbp->chead.cache.ref[sbp->chead.cache.in] = ref[sbp->chead.cluster_idx];
                ref[sbp->chead.cluster_idx] = NULL_REFERENCE;
                sbp->chead.cluster_idx++;
                if(sbp->chead.cluster_idx == soGetRPC()-1){
                    soWriteCluster(sbp->chead.cluster_number, buf);
                    soFreeCluster(sbp->chead.cluster_number);
                    sbp->chead.cluster_number = ref[sbp->chead.cluster_idx];
                    sbp->chead.cluster_idx = 0;
                    soReadCluster(sbp->chead.cluster_number, buf);
                    ref = (uint32_t *) buf;
                }
                sbp->chead.cache.in = (sbp->chead.cache.in + 1) % FCT_CACHE_SIZE;
                if(ref[sbp->chead.cluster_idx] == NULL_REFERENCE){
                    soWriteCluster(sbp->chead.cluster_number, buf);
                    soFreeCluster(sbp->chead.cluster_number);
                    sbp->chead.cluster_idx = sbp->ctail.cluster_idx = 0;
                    sbp->chead.cluster_number = sbp->ctail.cluster_number = NULL_REFERENCE;
                    sbp->crefs = 0;
                    break;
                }
            }

            if(sbp->crefs != 0) 
                soWriteCluster(sbp->chead.cluster_number, buf);
        }
        sbp->chead.cache.out = 0;
    }
    sbSave();
}
