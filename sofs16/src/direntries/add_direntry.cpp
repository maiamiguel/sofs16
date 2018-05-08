/**
 *  \author Daniel Alves 76469
 *  \tester ...
 */

#include "direntries.h"
#include <sys/stat.h>
#include "probing.h"
#include "exception.h"
#include <czdealer.h>
#include <filecluster.h>
#include <itdealer.h>
#include <errno.h>

/* assumes that:
 * - pih is a valid inode handler of a directory where the user has write access
 * - name can already exist or not (that should be tested)
 * - cin is a valid inode handler
 */
void soAddDirEntry(int pih, const char *name, uint32_t cin)
{
    soProbe(500, "soAddDirEntry(%d, %s, %u)\n", pih, name, cin);

    //throw SOException(ENOSYS, __FUNCTION__);

    SOInode* inode;						/* Variável que guarda o inode pai */
    bool entryFound = false;			/* Variável para guardar se foi encontrada */
    uint32_t dpc = soGetDPC();			/* Variável que guarda a informação do número de referências por cluster (valor por defeito: 16) */
   	SODirEntry buf [dpc];				/* Funciona como buffer e recebe o conteúdo de um cluster */

   	inode = iGetPointer(pih);			/* Carregar o inode*/ 

   	if((inode->mode & S_IFMT) != S_IFDIR) 							/*Verificar se inode pai é do tipo diretório */
    	throw SOException(ENOTDIR, __FUNCTION__);

    uint32_t dirRpi= inode->size / sizeof(SODirEntry);	/*Cálculo do número de referências do tipo SODirEntry que pode haver num cluster */

    uint32_t last_cluster_idx = dirRpi / dpc;			/*Cálculo do indice do último cluster utilizado*/

	/* Carregar cada cluster e percorre-lo para verificar se já existe uma entrada com o nome name*/
	for (uint32_t i = 0; i <= last_cluster_idx; i++)
    {
	    
        soReadFileCluster(pih,i,buf);					/* Carregar o cluster do bloco i associaddo ao inode*/
        
        /*Percorrer o cluster à procura da entrada */
        for (uint32_t j = 0; j < dpc; j++)
        {
        	/*Verificar para cada entrada se o nome é igual a name*/
            if (strcmp(buf[j].name,name) == 0)
            {
            entryFound = true;
            break;										/*Se encontrar a entrada para imediatmente*/
            }
		}
		/*Se encontrou a entrada não precisa de percorrer mais clusters e é lançada uma exceção*/
		if(entryFound)
			throw SOException(EEXIST, __FUNCTION__);
		else if(i == last_cluster_idx)
		{
		    
		    uint32_t last_entry_idx = dirRpi % dpc;		/*Cálculo da primeira posição livre no último cluster*/
		    
		    /*Guradar a nova referência na última posição*/
			strncpy(buf[last_entry_idx].name, name, SOFS16_MAX_NAME+1);
			buf[last_entry_idx].in = cin;
			
			/*Escrever o cluster de volta*/
			soWriteFileCluster(pih,last_cluster_idx,buf);
			
			/*Incrementar o tamanho do inode em bytes*/
		    inode->size += sizeof(SODirEntry);
		}
	}
}
