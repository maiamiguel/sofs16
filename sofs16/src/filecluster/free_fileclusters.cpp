/**

 *  \author Armando Sousa
 *  \tester Daniel Alves
 */

#include "filecluster.h"
#include "freelists.h"
#include "probing.h"
#include "exception.h"
#include "inode.h"
#include "core.h"
#include "cluster.h"
#include <dealers.h>
#include <errno.h>
#include <stdint.h>

static void soFreeIndirectFileClusters(SOInode * ip, uint32_t ffcn);
static void soFreeDoubleIndirectFileClusters(SOInode * ip, uint32_t ffcn);

/* ********************************************************* */

void soFreeFileClusters(int ih, uint32_t ffcn)
{
    soProbe(600, "soFreeFileClusters(%d, %u)\n", ih, ffcn);
    
	SOInode* ip = iGetPointer(ih);

	uint32_t rpc = soGetRPC();
	
	printf("RPC: %d\n",rpc);
	
	if (ffcn >= N_DIRECT + N_INDIRECT*rpc)
	{
		soFreeDoubleIndirectFileClusters(ip,ffcn - N_DIRECT - N_INDIRECT*rpc);		
	}
	else if (ffcn >= N_DIRECT)
	{
		soFreeIndirectFileClusters(ip,ffcn-N_DIRECT);
	}	
	else{
		
		for (uint32_t i = ffcn; i <N_DIRECT; i++)
		{
			if (ip->d[i] == NULL_REFERENCE)continue;
			soFreeCluster(ip->d[i]);
			ip->d[i] = NULL_REFERENCE;
			ip->csize--;
		}
		/*if (ip->i1[0] != NULL_REFERENCE)
		{*/
			soFreeIndirectFileClusters(ip,0);
		//}
	
	}
	
	/*Guardar o inode*/
	iSave(ih);
}
/* ********************************************************* */

/* only a hint to decompose the solution*/ 
static void soFreeIndirectFileClusters(SOInode * ip, uint32_t ffcn)
{
    soProbe(600, "soFreeIndirectFileClusters(%p, %u)\n", ip, ffcn);

    uint32_t rpc = soGetRPC();	/* Variável guarda a informação do número de referências por cluster (valor por defeito: 256) */
    uint32_t buf[rpc];			/* Funciona como buffer e recebe o conteúdo de um cluster */
    uint32_t *cluster;				
    uint32_t i1_index = ffcn/rpc;	/* Indice referente ao array i1, permite a alteração de N_DIRECT */
    uint32_t ref_index = ffcn%rpc;	/* Indice referente à posição da referencia a allocar */
    
    while(i1_index < N_INDIRECT && ip->i1[i1_index] != NULL_REFERENCE)
    {
    	/* Carregar a referência do cluster no buffer */
		soReadCluster(ip->i1[i1_index], buf);

		/* Fazer cast do buffer para tipo apropriado */
		cluster = (uint32_t *) buf;
		for(uint32_t i = ref_index; i < rpc; i++)
		{
			if(cluster[i] == NULL_REFERENCE) continue;
			soFreeCluster(cluster[i]);
			cluster[i] = NULL_REFERENCE;
			ip->csize--;
		}
		
		/* Guardar as referências do cluster no buffer */
		soWriteCluster(ip->i1[i1_index], cluster);

		if(ref_index == 0) 
		{
			/* Libertar o cluster usado para guradar as referências */
			soFreeCluster(ip->i1[i1_index]);
			ip->i1[i1_index] = NULL_REFERENCE;
			ip->csize--;
		}

		/* Incrementar o index dentro do array i1*/
		i1_index++;

		/* Passando ao próximo index começando a remoção do index 0*/
		ref_index = 0;
    }

    if (ip->i2 != NULL_REFERENCE)
	{
		soFreeDoubleIndirectFileClusters(ip, 0);
	}
}

/* ********************************************************* */

/* only a hint to decompose the solution */

static void soFreeDoubleIndirectFileClusters(SOInode * ip, uint32_t ffcn)
{
    soProbe(600, "soFreeDoubleIndirectFileClusters(%p, %u)\n", ip, ffcn);

    uint32_t rpc = soGetRPC();	/* Variável guarda a informação do número de referências por cluster (valor por defeito: 256) */
    uint32_t buf1[rpc];			/* Funciona como buffer e recebe o conteúdo de um cluster */
	uint32_t buf2[rpc];			/* Funciona como buffer e recebe o conteúdo de um cluster */
    uint32_t *i2_cluster;
    uint32_t *cluster;				
    uint32_t cluster_index = ffcn/rpc;	/* Indice referente à posição da primeira referência */
    uint32_t ref_index = ffcn%rpc;		/* Indice referente à posição da segunda referência */
    
    /* Carregar a referência do cluster presente em i2 no buffer */
    soReadCluster(ip->i2, buf1);

	/* Fazer cast do buffer para tipo apropriado */
	i2_cluster = (uint32_t *) buf1;

    while(cluster_index < rpc && i2_cluster[cluster_index] != NULL_REFERENCE)
    {
    	/* Carregar a referência do cluster no buffer */
		soReadCluster(i2_cluster[cluster_index], buf2);
		
		/* Fazer cast do buffer para tipo apropriado */
		cluster = (uint32_t *) buf2;

		for(uint32_t i = ref_index; i < rpc; i++)
		{
			if(cluster[i]==NULL_REFERENCE) continue;
			soFreeCluster(cluster[i]);
			cluster[i] = NULL_REFERENCE;
			ip->csize--;
		}	
		/* Guardar as referências do cluster no buffer */
		soWriteCluster(i2_cluster[cluster_index], cluster);

		if(ref_index == 0) 
		{
			/* Libertar o cluster usado para guradar as referências */
			soFreeCluster(i2_cluster[cluster_index]);
			i2_cluster[cluster_index] = NULL_REFERENCE;
			ip->csize--;
		}
		
		/* Incrementar o index dentro do cluster (i2) que guarda as referências */
		cluster_index++;

		/* Passando ao próximo index começando a remoção do index 0*/
		ref_index = 0;
    }

	/* Guardar as referências do cluster no buffer */
	soWriteCluster(ip->i2, i2_cluster);
	
	if(ffcn == 0)
	{
		/* Libertar o cluster usado para guradar as referências */
		soFreeCluster(ip->i2);
		ip->i2 = NULL_REFERENCE;
		ip->csize--;		
	}
}

/* ********************************************************* */
