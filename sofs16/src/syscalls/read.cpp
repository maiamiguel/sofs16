/*
 * \author Nelson Costa
 * \tester Daniel Alves
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

#include "syscalls.h"

#include "probing.h"
#include "exception.h"
#include "direntries.h"
#include "filecluster.h"
#include "itdealer.h"
#include "czdealer.h"
#include "core.h"

/*
 *  \brief Read data from an open regular file.
 *
 *  It tries to emulate <em>read</em> system call.
 *
 *  \param path path to the file
 *  \param buff pointer to the buffer where data to be read is to be stored
 *  \param count number of bytes to be read
 *  \param pos starting [byte] position in the file data continuum where data is to be read from
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soRead(const char *path, void *buff, uint32_t count, int32_t pos)
{
    soProbe(229, "soRead(\"%s\", %p, %u, %u)\n", path, buff, count, pos);

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
        
        /* Verificar o caminho devolve um inode válido */
        if(in == NULL_REFERENCE)                       
            throw SOException(EINVAL, __FUNCTION__);

        /* Abrir um handler para o inode */
        uint32_t ih = iOpen(in);            

        /* Variável que guarda o inode */
        SOInode* inode;

        /* Carregar o inode*/
        inode = iGetPointer(ih);             

        /* Verificar se inode é do tipo REG_FILE */
        if((inode->mode & S_IFREG) != S_IFREG)
            throw SOException(EINVAL, __FUNCTION__);

        /* Variável que guarda o número de bytes por cluster*/
        uint32_t bpc = soGetBPC();          
        
        /* Calcular o file cluster index correspondente à posição onde será iniciada a leitura*/
        uint32_t fcn = pos / bpc;

        /* Calcular o indice dentro do cluster correspondente à posição onde será iniciada a leitura*/
        uint32_t data_index = pos % bpc;

        /* Buffer para onde carregar o conteúdo do cluster */        
        char cluster[bpc];
        
        /* Limpar o buffer*/        
        memset(cluster, 0, bpc);

        /* Carregar a referência do cluster no buffer */
        soReadFileCluster(ih, fcn, cluster);

        char *ref;
        ref = (char *) buff;

        uint32_t i;
        /* Percorrer o(s) cluster(s) a serem lidos
        A iteração é feita byte a byte*/
        for(i = 0; i < count; i++)
        {
            /* Copiar o byte do buffer para ref*/
            ref[i] = cluster[data_index];

            /* Incrementar o indice dentro do cluster para o próximo a ser lido*/              
            data_index = (data_index + 1) % bpc;

            /* Quando indice chega à última posição no cluster passa para 0, é preciso carregar outro cluster*/
            if(data_index == 0)
            {   
                /* Limpar o buffer*/
                memset(cluster, 0, bpc);

                /* Carregar a referência do cluster no buffer */
                soReadFileCluster(ih, ++fcn, cluster);
            }
        }
        //buff = ref;
        iClose(ih);
        /* Operação termina com sucesso*/
        return i;
    }
    catch(SOException & err)
    {
        return -err.en;
    }
}
