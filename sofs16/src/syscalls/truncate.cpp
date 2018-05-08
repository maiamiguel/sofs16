/*
 * \author Miguel Maia 76434
 * tester: Armando Sousa
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
#include "dealers.h"
#include "freelists.h"
#include "filecluster.h"

/*
 *  \brief Truncate a regular file to a specified length.
 *
 *  It tries to emulate <em>truncate</em> system call.
 *
 *  \param path path to the file
 *  \param length new size for the regular size
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soTruncate(const char *path, off_t length){
    soProbe(231, "soTruncate(\"%s\", %u)\n", path, length);

    try{
        int perm = soAccess(path, (W_OK | R_OK));                   // verificar permissões do ficheiro
        if (perm != 0){
            return -perm;
        }

        char* xpath = strdupa(path);            

        uint32_t inp;
        soTraversePath(xpath, &inp);         
        int ih = iOpen(inp);
        SOInode* inode = iGetPointer(ih);
        
        if (S_ISREG(inode->mode)){                                   // verificar se o ficheiro é um ficheiro regular
            uint32_t cluster_size = soGetBPC();
            uint32_t cluster_number = length / cluster_size;         // determinar o nº do cluster apartir do qual é preciso começar a apagar/adicionar 
            uint32_t rem_bytes = length % cluster_size;              // restantes bytes

            uint32_t size = cluster_size / sizeof(uint32_t);         // tamanho do ficheiro atual
            
            if (inode->size > length){                           
                soFreeFileClusters(ih,  (cluster_number + 1)); // libertar os clusters desnecessários
                uint32_t buffer[size];

                soReadFileCluster(ih, inode->csize-1, &buffer); //ler a informação do cluster

                uint32_t start = rem_bytes / sizeof(uint32_t) + 1;    // "eliminar" a parte extra
                for (; start < size; start++ ){
                    buffer[start] = 0x00000000;
                }
                soWriteFileCluster(ih, inode->csize-1, &buffer); // guardar as alterações

            }
            else if(inode->size < length){                             // se o ficheiro for menor que o tamanho que se pretende
                uint32_t buffer[size];
                soReadFileCluster(ih, inode->csize-1, &buffer);  // ler o cluster

                if (cluster_number + 1 != inode->csize){
                    uint32_t start = (inode->size % cluster_size) / sizeof(uint32_t);   
                    while (start < size){
                        buffer[start++] = '\0';                         
                    }

                    soWriteFileCluster(ih, inode->csize-1, buffer); // guardar as alterações

                    uint32_t i = 0;
                    while (i < size){
                        buffer[i] = '\0';
                        i++;
                    }

                    while (inode->csize < (cluster_number+1)  ){
                        soWriteFileCluster(ih, inode->csize, &buffer);
                    }
                }
                else{
                    uint32_t start = (inode->size % cluster_size) / sizeof(uint32_t);
                    uint32_t end = rem_bytes / sizeof(uint32_t);
                    while (start < end){
                        buffer[start++] = '\0';
                    }
                    soWriteFileCluster(ih, inode->csize-1, buffer);
                }
            }
            
            inode->atime = time(NULL);
            inode->mtime = time(NULL);
            inode->size = length;
        }
        else{
            throw SOException(-EINVAL,__FUNCTION__);                               // caso o ficheiro não seja um ficheiro regular                         
        }

        iSave(ih);
        iClose(ih);
        return 0;
    }
    catch (SOException & err){
        return -err.en;
    }
}
