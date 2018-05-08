/*
 * \author Miguel Maia 76434
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
#include "core.h"
#include "direntries.h"
#include "itdealer.h"
#include "freelists.h"

/*
 *  \brief Create a regular file with size 0.
 *
 *  It tries to emulate <em>mknod</em> system call.
 *
 *  \param path path to the file
 *  \param mode type and permissions to be set:
 *                    a bitwise combination of S_IFREG, S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH,
 *                    S_IWOTH, S_IXOTH
 *
 *  \return 0 on success;
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soMknod(const char *path, mode_t mode){
    soProbe(228, "soMknod(\"%s\", %u)\n", path, mode);

    try{
        uint32_t cInodeHandler;
        char* xpath = strdupa (path);           // duplica o array de chars "path"
        char* bn = strdupa(basename(xpath));    // nome da direntry a ser criada
        char* dn = dirname(xpath);              // nome/path do diretorio onde se quer criar o novo diretorio

        uint32_t pInodeNumber;                  // pInodeNumber = parent inode number do ficheiro que se quer criar
        soTraversePath(dn, &pInodeNumber);      // o nº do inode obtido pelo traverse irá ser guardado na variavel "pInodeNumber"
                                                // Devolve nº do inode caso encontre o diretorio com o nome/ path "dn"
                                                // Devolve N/R se nao encontrar

        // verificar se o pInodeNumber é N/R; se for significa q o traverse_path nao encontrou o diretório onde se quer criar o novo ficheiro
        if (pInodeNumber == NULL_REFERENCE){
            throw SOException(ENOENT,__FUNCTION__);
        }

        // caso encontremos o inode correspondente ao diretorio pai, q é onde queremos criar o ficheiro, prosseguimos
        // Verificar se existe um direntry no parent inode que possui o mesmo nome que o ficehiro a ser criado
        uint32_t cInodeNumber;                                     // irá guardar o inode number caso este seja obtido no soGetDirEntry()
        int pInodeHandler = iOpen(pInodeNumber);                   // parent inode handler
        soGetDirEntry(pInodeHandler, bn, &cInodeNumber);

        if (cInodeNumber != NULL_REFERENCE){             		   // significa q existe uma entrada com o nome do ficheiro que se quer criar
            throw SOException(EEXIST,__FUNCTION__);
        }
        // caso nao exista nenhum direntry com o mesmo nome prossegue-se

        SOInode* parentInode = iGetPointer(pInodeHandler);

        if ((parentInode->mode && S_IFDIR == S_IFDIR) || (parentInode->mode && S_IFLNK == S_IFLNK) ){ // verificar se o inode parent corresponde a um diretorio ou atalho
        													 // alocar o inode para o novo diretorio
            soAllocInode(S_IFREG, &cInodeNumber);  			 // devolve o nº do inode, guardando-o na variavel cInodeNumber
            cInodeHandler = iOpen(cInodeNumber);
            iSetAccess(cInodeHandler,(uint16_t)mode);
            soAddDirEntry(pInodeHandler,bn,cInodeNumber);    //adicionar o direntry, no parent inode, correspondente ao ficheiro q se está a criar

            iIncRefcount(cInodeHandler);  					 //incrementar o refcount no parent inode, correspondente ao novo inode

            iClose(pInodeHandler);  						 //close parent inode
        }
        else{
            throw SOException(ENOTDIR,__FUNCTION__);
        }
        return 0;
    }
    catch(SOException & err){
        return -err.en;
    }
}