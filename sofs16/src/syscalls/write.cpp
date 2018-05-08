/*
 * \author Daniel Alves
 * \tester Nelson Costa
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <libgen.h>
#include <string.h>

#include "direntries.h"
#include "itdealer.h"
#include "core.h"
#include "syscalls.h"
#include "czdealer.h"
#include "filecluster.h"
#include "probing.h"
#include "exception.h"

/*
 *  \brief Write data into an open regular file.
 *
 *  It tries to emulate <em>write</em> system call.
 *
 *  \param path path to the file
 *  \param buff pointer to the buffer where data to be written is stored
 *  \param count number of bytes to be written
 *  \param pos starting [byte] position in the file data continuum where data is to be written into
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soWrite(const char *path, void *buff, uint32_t count, int32_t pos)
{
    soProbe(230, "soWrite(\"%s\", %p, %u, %u)\n", path, buff, count, pos);

    try
    {
        /* substitute this throw by your code */
        /* return -ENOSYS;*/

        /* Verificar se o caminho é válido, devolve um inode válido e tem permissões*/
        uint32_t in;

        /* Criar um duplicado do path que possa ser passado à função soTraversePath*/
        char *xpath = strdupa(path);

        /* Receber o número do inode associado ao caminho path*/
        soTraversePath(xpath, &in);
        
        /*Verificar o caminho devolve um inode válido */
        if(in == NULL_REFERENCE)                           
            throw SOException(EINVAL, __FUNCTION__);

        /* Abrir um handler para o inode */
        uint32_t ih = iOpen(in);            

        /* Variável que guarda o inode */
        SOInode* inode;

        /* Carregar o inode*/
        inode = iGetPointer(ih);             

        /*Verificar se inode é do tipo REG_FILE */
        if((inode->mode & S_IFREG) != S_IFREG)
            throw SOException(EINVAL, __FUNCTION__);
        uint32_t initial_size = inode->size;

        /* Fazer cast do buffer para tipo apropriado */
        char *ref;  
        ref = (char *) buff;

        /* Variável que guarda o número de bytes por cluster*/
        uint32_t bpc = soGetBPC();          
        
        /* Calcular o file cluster index correspondente à posição onde será iniciada a escrita*/
        uint32_t fcn = pos / bpc;

        /* Calcular o indice dentro do cluster correspondente à posição onde será iniciada a escrita*/
        uint32_t data_index = pos % bpc;
        
        /* Buffer para onde carregar o conteúdo do cluster */        
        char cluster[bpc];
        
        /* Limpar o buffer*/        
        memset(cluster, 0, bpc);

        /* Carregar a referência do cluster no buffer */
        soReadFileCluster(ih, fcn, cluster);

        /* Percorrer o(s) cluster(es) a serem escritos
        A iteração é feita byte a byte*/
        for(uint32_t i = 0; i < count; i++)
        {
            /* Copiar o byte para o buffer*/
            cluster[data_index] = ref[i]; 

            /* Incrementar o index dentro do cluster para o próximo a ser escrito*/              
            data_index = (data_index + 1) % bpc;

            /* Incrementar o tamanho do inode em bytes*/
            inode->size += sizeof(uint8_t);

            /* Quando index chega à última posição no cluster passa para 0, pelo que é preciso guardar
            o cluster em uso e carregar outro*/
            if(data_index == 0 || i == count - 1)
            {
                /* Guardar as referências do cluster no buffer */
                soWriteFileCluster(ih, fcn, cluster);
                
                /* Limpar o buffer*/
                memset(cluster, 0, bpc);
                
                if(i != count - 1)
                {
                    /* Carregar a referência do cluster no buffer */
                    soReadFileCluster(ih, ++fcn, cluster);
                }
                
            }
        }
        iSave(ih);
        iClose(ih);
        /* Operação termina com sucesso*/
        return inode->size - initial_size;
    }
    catch(SOException & err)
    {
        return -err.en;
    }
}
