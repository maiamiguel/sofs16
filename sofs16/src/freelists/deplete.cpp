/**
 *  \author Daniel Alves
 *  \tester ...
 */

#include "freelists.h"

#include "probing.h"
#include "exception.h"
#include "freelists.h"
#include "superblock.h"
#include "sbdealer.h"
#include <errno.h>
#include "cluster.h"
#include "czdealer.h"
#include "core.h"

/*
 * Even if some of them are not functionally necessary,
 * the following dictates must be obyed by the implementation:
 * - if crefs is equal to zero, 
 *      first transfer as much as possible to head cache
 * - 
 */
/**
 * \brief Deplete the tail cache
 *
 * The references in the tail cache are transferred to the head cache or to
 *      the references tail cluster.
 */

void soDeplete(void)
{
    soProbe(722, "soDeplete()\n");
    
    //throw SOException(ENOSYS, __FUNCTION__);

    SOSuperBlock* sbp;
    sbp = sbGetPointer();
    
    uint32_t bpc = soGetBPC();      /* Variável guarda a informação do número de bytes por cluster (valor por defeito: 1024) */
    uint32_t rpc = soGetRPC();	/* Variável guarda a informação do número de referências por cluster (valor por defeito: 256) */
    uint32_t buf[rpc];			/* Funciona como buffer e recebe o conteúdo de um cluster */
    uint32_t buf2[rpc];          /* Funciona como buffer e recebe o conteúdo de um cluster */		
    uint32_t cnp;				/* Ponteiro para o número do cluster*/

    /* Se a tail cache estiver vazia não faz nada */
    if (sbp->ctail.cache.in == sbp->ctail.cache.out && sbp->ctail.cache.out == NULL_REFERENCE)
        return;

    /* Verificar se a Free Cluster List está vazia */
    if (sbp->crefs == 0) 	
    {					
    	/* Se estiver vazia efetuar transferência dos valores da tail cache para a head cache enquanto houver espaço*/
        while(sbp->chead.cache.in != sbp->chead.cache.out)
    	{
	    	/* Copiar elementos da tail cache para a primeira posição disponivel na head cache */
	    	sbp->chead.cache.ref[sbp->chead.cache.in] = sbp->ctail.cache.ref[sbp->ctail.cache.out];
	    	
	    	/* Colocar uma referência a NULL na posição da tail cache que se acabou de copiar */
	    	sbp->ctail.cache.ref[sbp->ctail.cache.out] = NULL_REFERENCE;
	    	sbp->ctail.cache.out = (sbp->ctail.cache.out + 1)%FCT_CACHE_SIZE;
	    	
	    	sbp->chead.cache.in =(sbp->chead.cache.in+1)%FCT_CACHE_SIZE;
	    	/* Controlo para verificar se ainda existem referências na tail cache*/
	    	if(sbp->ctail.cache.in == sbp->ctail.cache.out) break;
    	}
    	/* Se o espaço disponível na head cache for insuficiente é necessário alocar um cluster para a Free Cluster List,
    	que guarde a informação dos cluster para qual não houve espaço na head cache*/
    	if(sbp->ctail.cache.in != sbp->ctail.cache.out)
    	{
    		/* Alocar do primeiro cluster livre disponível */
    		soAllocCluster(&cnp);

    		/* Atualizar a informação no superbloco. Como só existe um cluster na FCT o head e o tail cluster são o mesmo*/
    		sbp->crefs++;
    		sbp->chead.cluster_number = cnp;
    		sbp->ctail.cluster_number = cnp;
    		sbp->chead.cluster_idx = 0;
    		sbp->ctail.cluster_idx = 0;
    		
            /* Cluster iniciaçizado com refercias as NULL_REFERENCE*/
            memset(buf, NULL_REFERENCE, bpc);

            /* Guardar as referências do cluster no buffer */
            soWriteCluster(sbp->ctail.cluster_number, buf);

    		/* Carregar a referência do cluster no buffer */
			soReadCluster(sbp->ctail.cluster_number, buf);

			/* Escrever as referências que sobram no cluster.*/
    		do
    		{
				buf[sbp->ctail.cluster_idx] = sbp->ctail.cache.ref[sbp->ctail.cache.out];
				/*Colocar uma referência a NULL na posição da tail cache que se acabou de copiar*/
				sbp->ctail.cache.ref[sbp->ctail.cache.out] = NULL_REFERENCE;
				sbp->ctail.cache.out = (sbp->ctail.cache.out+1)%FCT_CACHE_SIZE;
				sbp->ctail.cluster_idx++;
    		}
            while(sbp->ctail.cache.out != sbp->ctail.cache.in);
    		/* Guardar as referências do cluster no buffer */
			soWriteCluster(sbp->ctail.cluster_number, buf);
    	}
    }
    /* Se a Free Cluster List não está vazia a informação na tail cache deve ser copiada para o final do tail cluster */
    else
    {
    	/* Carregar a referência do cluster no buffer */
		soReadCluster(sbp->ctail.cluster_number, buf);
		
    	do
        {
    		/* Se o cluster ficar cheio, alocar outro cluster e escrever a referência para esse novo cluster no final do anterior*/
    		if(sbp->ctail.cluster_idx == rpc -1)
    		{
    			/* Alocar do primeiro cluster livre disponível */
    			soAllocCluster(&cnp);
    	        
    			buf[rpc -1] = cnp;

    			/* Guardar as referências do cluster no buffer */
    			soWriteCluster(sbp->ctail.cluster_number, buf);
    			
                sbp->crefs++;
    			sbp->ctail.cluster_number = cnp;
    	        
                /* Cluster iniciaçizado com refercias as NULL_REFERENCE*/
                memset(buf2, NULL_REFERENCE, bpc);

                /* Guardar as referências do cluster no buffer */
                soWriteCluster(sbp->ctail.cluster_number, buf2);

    			sbp->ctail.cluster_idx = 0;
    	
    			/* Carregar a referência do cluster no buffer */
				soReadCluster(sbp->ctail.cluster_number, buf);
    		}
    		/* Colocar o número do cluster alocado na referência correspondente */
			buf[sbp->ctail.cluster_idx] = sbp->ctail.cache.ref[sbp->ctail.cache.out];
			
			/* Colocar uma referência a NULL na posição da tail cache que se acabou de copiar */
			sbp->ctail.cache.ref[sbp->ctail.cache.out] = NULL_REFERENCE;
			sbp->ctail.cache.out =(sbp->ctail.cache.out+1)%FCT_CACHE_SIZE;
			sbp->ctail.cluster_idx++;
            /* Guardar as referências do cluster no buffer */
            
    	}
        while(sbp->ctail.cache.out != sbp->ctail.cache.in);
        soWriteCluster(sbp->ctail.cluster_number, buf);
	}
    
    /* Guardar o superblock*/    
    sbSave();
}
