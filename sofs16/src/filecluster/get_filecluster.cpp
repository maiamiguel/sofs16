/**
 *  \author Vitor Morais
 *  \tester ...
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


static void soGetIndirectFileCluster(SOInode * ip, uint32_t fcn, uint32_t * cnp);
static void soGetDoubleIndirectFileCluster(SOInode * ip, uint32_t fcn, uint32_t * cnp);


/* ********************************************************* */

void soGetFileCluster(int ih, uint32_t fcn, uint32_t * cnp)
{
 
    soProbe(600, "soGetFileCluster(%d, %u, %p)\n", ih, fcn, cnp);

    //throw SOException(ENOSYS, __FUNCTION__);
    				
    SOInode* ip;
    uint32_t rpc = soGetRPC();	/* Variável guarda a informação do número de referências por cluster (valor por defeito: 256) */


    /*Carregar ponteiro para o inode*/
	ip = iGetPointer(ih);

	/* Verificação da zona de clusters na qual estara o cluster pretendido */
	if(fcn < N_DIRECT)
	{
		/* Colocar referencia direta para o data cluster */
   		*cnp =ip->d[fcn];
		
	}
	else if(fcn < N_INDIRECT*rpc + N_DIRECT)
	{
		/* Esta na zona indireta */
		/* A função recebe o valor do file cluster index ajustado para a zona indireta*/
		soGetIndirectFileCluster(ip, fcn - N_DIRECT, cnp);
	}
	else
	{
		/* Esta na zona duplamente indireta */
		/* A função recebe o valor do file cluster index ajustado para a zona duplamente indireta*/
		soGetDoubleIndirectFileCluster(ip, fcn - N_DIRECT - N_INDIRECT*rpc, cnp);
	}
	/*Guardar o inode*/
	iSave(ih);
}



/* only a hint to decompose the solution */
static void soGetIndirectFileCluster(SOInode * ip, uint32_t afcn, uint32_t * cnp)
{
    soProbe(600, "soGetIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);
				
    

    uint32_t rpc = soGetRPC();	/* Variável guarda a informação do número de referências por cluster (valor por defeito: 256) */
    uint32_t buf[rpc];			/* Funciona como buffer e recebe o conteúdo de um cluster */
    uint32_t *ref;				
    uint32_t i1_index = afcn/rpc;	/* Indice referente ao array i1, permite a alteração de N_DIRECT */
    uint32_t ref_index = afcn%rpc;	/* Indice referente à posição da referencia a allocar */

    
    if (ip->i1[i1_index]!= NULL_REFERENCE){
	/* Carregar a referência do cluster no buffer */
	soReadCluster(ip->i1[i1_index], buf);
	
	/* Fazer cast do buffer para tipo apropriado */
	ref = (uint32_t *) buf;
	
	
	
	/* Colocar o número do cluster pretendido na referência correspondente */
	*cnp = ref[ref_index]; 
}
	
	else 
		*cnp=NULL_REFERENCE;

}

/* ********************************************************* */

/* only a hint to decompose the solution */
static void soGetDoubleIndirectFileCluster(SOInode * ip, uint32_t afcn, uint32_t * cnp)
{
    soProbe(600, "soGetDoubleIndirectFileCluster(%p, %u, %p)\n", ip, afcn, cnp);
	
    uint32_t rpc = soGetRPC();	/* Variável guarda a informação do número de referências por cluster (valor por defeito: 256) */
    uint32_t buf[rpc];			/* Funciona como buffer e recebe o conteúdo de um cluster */
    uint32_t *ref;			
    uint32_t cluster_index = afcn/rpc;	/* Indice referente à posição da primeira referência */
    uint32_t ref_index = afcn%rpc;		/* Indice referente à posição da segunda referência */
    uint32_t ref_clust_idx;				/* Váriável temporária para guardar cluster_index para o processo de chamada da segunda referência */


	/* Verificar se existem referências na variável i2 */
   if(ip->i2 != NULL_REFERENCE){
		/* Carregar a referência do cluster no buffer */
		soReadCluster(ip->i2, buf);

		/* Fazer cast do buffer para tipo apropriado */
		ref = (uint32_t *) buf;

		/* Colocar o número do cluster pretendido na referência correspondente */
		/* Guardar o valor do indice para a segunda referância */
		ref_clust_idx = ref[cluster_index];
		
		/* Carregar a referência do cluster no buffer */
		soReadCluster(ref_clust_idx, buf);
		
		/* Fazer cast do buffer para tipo apropriado */
		ref = (uint32_t *) buf;

		/* Colocar o número do cluster pretendido na referência correspondente */
		*cnp = ref[ref_index]; 
	}
	else
		*cnp=NULL_REFERENCE;
	

}


