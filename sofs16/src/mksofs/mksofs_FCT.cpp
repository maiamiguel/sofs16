/**
 *  \author Miguel Maia 76434
 *  \tester Miguel Maia 76434
 */
#include "mksofs.h"
#include "superblock.h"
#include "exception.h"
#include <errno.h>
#include <core.h>
#include <cluster.h>
#include <stdio.h>

/*
 * create the table of references to free data clusters 
 */
void fillInFreeClusterList(SOSuperBlock * p_sb){   
    uint32_t rpc = RPB * p_sb->csize;               // references per cluster
    uint32_t i;
    uint32_t clust[rpc];
    uint32_t nf = p_sb->crefs + 1;                  // next free cluster
    uint32_t sz = p_sb->czstart;                    // starting zone (block)
    uint32_t cl = 1;

    while (cl <= p_sb->crefs){
        for (i = 0; i < rpc-1; i++){            // Percorrer o cluster 
            if (nf <= p_sb->ctotal - 1){        // condição para verificar se há clusters disponíveis
                clust[i] = nf;                  
                nf++;                           // Next free cluster is the next cluster
            }
            else{
                clust[i] = NULL_REFERENCE;      // caso não haja referencias preenche com null
            }
        }
        if (cl == p_sb->crefs){    
            clust[rpc-1] = NULL_REFERENCE;      // Como este é o último cluster, então não aponta para o seguinte,
                                                // assim, a sua última posição, é igual a NULL_REFERENCE
        }
        else{
            clust[rpc-1] = cl + 1;              // "aponta" para o próximo cluster, ou seja, a última posição deste cluster
                                                // irá conter a referência para o cluster seguinte
        }
        soWriteRawCluster(sz + cl*p_sb->csize,clust,p_sb->csize);
                                                // Necessário guardar a informação no cluster
        cl++;                                   // Próximo cluster
    }
}