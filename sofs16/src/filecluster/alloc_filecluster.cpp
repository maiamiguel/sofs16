/**
 *  \author Daniel Alves
 *  \tester Nelson Costa
 */
#include "freelists.h"
#include "filecluster.h"
#include "cluster.h"
#include "core.h"
#include "probing.h"
#include "exception.h"
#include "inode.h"
#include "superblock.h"
#include "sbdealer.h"
#include "czdealer.h"
#include "itdealer.h"
#include <errno.h>
#include <stdint.h>

/*
 *Associate a cluster to a given file cluster position.
 *	Parameters
 *		- ih	inode handler
 *		- fcn	file cluster number
 *		- cnp	pointer to the variable where the cluster number must be put
*/

static void soAllocIndirectFileCluster(SOInode * ip, uint32_t fcn, uint32_t * cnp);
static void soAllocDoubleIndirectFileCluster(SOInode * ip, uint32_t fcn, uint32_t * cnp);

/* ********************************************************* */

void soAllocFileCluster(int ih, uint32_t fcn, uint32_t * cnp)
{
    soProbe(600, "soAllocFileCluster(%d, %u, %p)\n", ih, fcn, cnp);

    //throw SOException(ENOSYS, __FUNCTION__);
    			
    SOInode* ip;
    uint32_t rpc = soGetRPC();	/* Variável guarda a informação do número de referências por cluster (valor por defeito: 256) */

    /*Carregar ponteiro para o inode*/
	ip = iGetPointer(ih);

	/* Verificação da zona de clusters na qual será guardada a informação relativa o cluster do ficheiro */
	if(fcn < N_DIRECT)
	{
		/* Alocar na zona direta */
		/* Alocar do primeiro cluster livre disponível */
		soAllocCluster(cnp);

		/* Colocar referencia direta para alocar o número do data cluster */
		ip->d[fcn] = *cnp;

		/* Incrementar o número total de clusters usados no inode */
		ip->csize++;	
	}
	else if(fcn < N_INDIRECT*rpc + N_DIRECT)
	{
		/* Alocar na zona indireta */
		/* A função recebe o valor do file cluster index ajustado para a zona indireta*/
		soAllocIndirectFileCluster(ip, fcn - N_DIRECT, cnp);
	}
	else
	{
		/* Alocar na zona duplamente indireta */
		/* A função recebe o valor do file cluster index ajustado para a zona duplamente indireta*/
		soAllocDoubleIndirectFileCluster(ip, fcn - N_DIRECT - N_INDIRECT*rpc, cnp);
	}
	/*Guardar o inode*/
	iSave(ih);

}


/* ********************************************************* */

/*only a hint to decompose the solution */
static void soAllocIndirectFileCluster(SOInode * ip, uint32_t afcn, uint32_t * cnp)
{
    soProbe(600, "soAllocIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);
   
    //throw SOException(ENOSYS, __FUNCTION__);
    uint32_t bpc = soGetBPC();		/* Variável guarda a informação do número de bytes por cluster (valor por defeito: 1024) */
    uint32_t rpc = soGetRPC();	    /* Variável guarda a informação do número de referências por cluster (valor por defeito: 256) */
    uint32_t buf[rpc];			    /* Funciona como buffer e recebe o conteúdo de um cluster */
    uint32_t i1_index = afcn/rpc;	/* Indice referente ao array i1, permite a alteração de N_DIRECT */
    uint32_t ref_index = afcn%rpc;	/* Indice referente à posição da referencia a allocar */

    /* Verificar se existem referências no indice do array i1 */
    /* Se não houver é necessário alocar */
    if (ip->i1[i1_index] == NULL_REFERENCE) 
	{
	   	/* Alocar do primeiro cluster livre disponível */
	   	soAllocCluster(cnp);

		/* Colocar referencia indireta para alocar o número do data cluster */
	  	ip->i1[i1_index] = *cnp;

	  	/* Cluster iniciaçizado com refercias as NULL_REFERENCE*/
	  	memset(buf, NULL_REFERENCE, bpc);

	  	/* Guardar as referências do cluster no buffer */
	  	soWriteCluster(ip->i1[i1_index], buf);

		/* Incrementar o número total de clusters usados no inode */
   		ip->csize++;	
	}
	/* Carregar a referência do cluster no buffer */
	soReadCluster(ip->i1[i1_index], buf);

	/* Alocar do primeiro cluster livre disponível */
	soAllocCluster(cnp);

	/* Incrementar o número total de clusters usados no inode */
   	ip->csize++;	

	/* Colocar o número do cluster alocado na referência correspondente */
	buf[ref_index] = *cnp;

	/* Guardar as referências do cluster no buffer */
	soWriteCluster(ip->i1[i1_index], buf);
 
}

/* ********************************************************* */

/* only a hint to decompose the solution */
static void soAllocDoubleIndirectFileCluster(SOInode * ip, uint32_t afcn, uint32_t * cnp)
{
    soProbe(600, "soAllocDoubleIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);

    //throw SOException(ENOSYS, __FUNCTION__);
	uint32_t bpc = soGetBPC();		/* Variável guarda a informação do número de bytes por cluster (valor por defeito: 1024) */
    uint32_t rpc = soGetRPC();	/* Variável guarda a informação do número de referências por cluster (valor por defeito: 256) */
    uint32_t buf[rpc];			/* Funciona como buffer e recebe o conteúdo de um cluster */		
    uint32_t buf2[rpc];
    uint32_t cluster_index = afcn/rpc;	/* Indice referente à posição da primeira referência */
    uint32_t ref_index = afcn%rpc;		/* Indice referente à posição da segunda referência */
    uint32_t ref_clust_idx;				/* Váriável temporária para guardar cluster_index para o processo de chamada da segunda referência */

	/* Verificar se existem referências na variável i2 */
    /* Se não houver é necessário alocar */
    if (ip->i2 == NULL_REFERENCE) {

		/* Alocar do primeiro cluster livre disponível */
	   	soAllocCluster(cnp);
		
		/* Colocar a primeira referencia indireta para alocar o número do data cluster */
	  	ip->i2 = *cnp;
		
		/* Cluster iniciaçizado com refercias as NULL_REFERENCE*/
		memset(buf, NULL_REFERENCE, bpc);
		
		/* Guardar as referências do cluster no buffer */
	  	soWriteCluster(ip->i2, buf);

		/* Incrementar o número total de clusters usados no inode */
   		ip->csize++;	
	}
	
	/* Carregar a referência do cluster no buffer */
	soReadCluster(ip->i2, buf);

	/* Verificar se existem referências na variável i2 */
    /* Se não houver é necessário alocar */
    if (buf[cluster_index] == NULL_REFERENCE) 
	{
		/* Alocar do primeiro cluster livre disponível */	
		soAllocCluster(cnp);

		/* Colocar o número do cluster alocado na referência correspondente */
		buf[cluster_index] = *cnp;

		/* Cluster iniciaçizado com refercias as NULL_REFERENCE*/
		memset(buf2, NULL_REFERENCE, bpc);

		/* Guardar as referências do cluster no buffer */
	  	soWriteCluster(buf[cluster_index], buf2);
		/* Incrementar o número total de clusters usados no inode */
   		ip->csize++;
	}

	/* Guardar o valor do indice para a segunda referência */
	ref_clust_idx = buf[cluster_index];

	/* Guardar as referências do cluster no buffer */
	soWriteCluster(ip->i2, buf);	
	
	/* Carregar a referência do cluster no buffer */
	soReadCluster(ref_clust_idx, buf);

	/* Alocar do primeiro cluster livre disponível */	
	soAllocCluster(cnp);

	/* Incrementar o número total de clusters usados no inode */
   	ip->csize++;
	
	/* Colocar o número do cluster alocado na referência correspondente */
	buf[ref_index] = *cnp;

	/* Guardar as referências do cluster no buffer */
	soWriteCluster(ref_clust_idx, buf);

}
