/**
 *  \author Miguel Maia 76434
 *  \tester Miguel Maia 76434
 */
#include "superblock.h"
#include "direntries.h"
#include "probing.h"
#include "exception.h"
#include <errno.h>
#include <czdealer.h>
#include <filecluster.h>
#include <itdealer.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <unistd.h>

/**
 *  \brief Remove an entry from a parent directory.
 *  A direntry associated from the given directory is deleted.
 *  The refcount of the child inode is not decremented by this function.
 *  \param pih inode handler of the parent inode
 *  \param name name of the entry
 *  \param cinp Pointer to the variable where the number of the child inode is to be stored
 */

void soDeleteDirEntry(int pih, const char *name, uint32_t * cinp){
    soProbe(500, "soDeleteDirEntry(%d, %s, %p)\n", pih, name, cinp);

    SOInode* inode;
    inode = iGetPointer(pih);                                               // guardar referência para o parent inode

    uint32_t dpc = soGetDPC();                                              // direntries per cluster
    
    SODirEntry info [dpc];                                                  
    bool direntryFound = false;

    uint32_t Nrefs,i,j,pos;
    Nrefs = inode->size / sizeof(SODirEntry);
    
    uint32_t last_clust_idx = Nrefs / dpc;                                     // Dá-nos o número do cluster onde se encontra a última posição
    uint32_t last_entry_idx = (Nrefs-1) % dpc;                                 // Posição da última direntry
    SODirEntry last_entry;
    soReadFileCluster(pih,last_clust_idx,info);
    last_entry = (SODirEntry) info[last_entry_idx];
        
    /*****************************/

    if (strlen(name) == 0 || name == NULL){
        throw SOException(EINVAL, __FUNCTION__);
    }

    if (strlen(name) > SOFS16_MAX_NAME+1){
    	throw SOException(EINVAL, __FUNCTION__);
    }

    if (!iCheckAccess(pih,X_OK)){
        throw SOException(EACCES, __FUNCTION__);
    } 

    for (i = 0; i < Nrefs; i++){
        pos = i/dpc;
        if ((i % dpc ) == 0){
            soReadFileCluster(pih,pos,info);        
            for (j = 0; j < dpc; j++){        
                if (strcmp(info[j].name,name) == 0){
                    *cinp = info[j].in;
                    info[j].in = last_entry.in;                                     
                    strncpy(info[j].name, last_entry.name, SOFS16_MAX_NAME+1);      
                    
                    inode->size -= sizeof(SODirEntry);
                    direntryFound = true;

                    info[last_entry_idx].in = NULL_BLOCK;                                     //APAGAR A ÚLTIMA ENTRADA
                    strncpy(info[last_entry_idx].name, "\0", SOFS16_MAX_NAME+1);              //APAGAR A ÚLTIMA ENTRADA

                    soWriteFileCluster(pih,last_clust_idx,info);

                    //Guardar as alterações, ou seja a última entrada já não existe, caso contrário seria duplicada

                    //Guardar as alterações feitas, neste caso já não contém a última entrada, pois quando se elimina a direntry pretendida, a última entrada deve permanecer na posição eliminada
                    //a direntry pretendida foi encontrada, agora é guardar as alterações
                    break;                                                                      //Não pode haver 2 direntries com o mesmo nome, logo job done
                }
            }
        }
        
        if (direntryFound){
            soWriteFileCluster(pih,pos,info);
            break;                                                // já se fez as alterações necessárias logo não é necessário continuar
        }
        else{
            throw SOException(ENOENT, __FUNCTION__);              // Se não se encontrou essa entrada significa que ela não existe, logo "No such file or directory"
        }
        
        if (last_entry_idx == 0){                                 //visto que o cluster apenas contém uma direntry ( a última)
            soFreeFileClusters(pih,last_clust_idx);               //é necessário libertar esse cluster    
        }       
    }
}