/*
 * \author Vitor Morais
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
 *  \brief Create a directory.
 *
 *  It tries to emulate <em>mkdir</em> system call.
 *
 *  \param path path to the file
 *  \param mode permissions to be set:
 *          a bitwise combination of S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failute
 */
int soMkdir(const char *path, mode_t mode)
{
    soProbe(232, "soMkdir(\"%s\", %u)\n", path, mode);

        char* xpath = strdupa (path);           // duplica o array de chars "path"
        char* bn = strdupa(basename(xpath));    // nome da direntry a ser criada
        char* dn = dirname(xpath);              // nome/path do diretorio onde se quer criar o novo diretorio

        uint32_t pInodeNumber;                  // pInodeNumber = parent inode number do directorio que se quer criar
        uint32_t cInodeNumber;   // child inode number
        uint32_t pInodeHandler;   // parent inode handler
        uint32_t cInodeHandler;   // child inode handler
    
    try
    {



        soTraversePath(dn, &pInodeNumber);      // o nº do inode obtido pelo traverse irá ser guardado na variavel "pInodeNumber"
                                                // Devolve nº do inode caso encontre o diretorio com o nome/ path "dn"
                                                // Devolve N/R se nao encontrar

        // verificar se o pInodeNumber é N/R; se for significa q o traverse_path nao encontrou o diretório onde se quer criar o novo directorio
        if (pInodeNumber == NULL_REFERENCE){
            throw SOException(ENOENT, "Directory not found");
        }

        // caso encontremos o inode correspondente ao diretorio pai, q é onde queremos criar o directorio, prosseguimos
        // Verificar se existe um direntry no parent inode que possui o mesmo nome que o directorio a ser criado                                    // irá guardar o inode number caso este seja obtido no soGetDirEntry()
        pInodeHandler = iOpen(pInodeNumber);                   // parent inode handler
        soGetDirEntry(pInodeHandler, bn, &cInodeNumber);

        if (cInodeNumber != NULL_REFERENCE){                       // significa q existe uma entrada com o nome do directorio que se quer criar
            throw SOException(EEXIST, "The file name specified already exists");
        }
        // caso nao exista nenhum direntry com o mesmo nome prossegue-se

        SOInode* parentInode = iGetPointer(pInodeHandler);


        if ((parentInode->mode && S_IFDIR == S_IFDIR)){ // verificar se o inode parent corresponde a um diretorio ou atalho
                                                             // alocar o inode para o novo diretorio
            soAllocInode((uint16_t)16384, &cInodeNumber);            // devolve o nº do inode, guardando-o na variavel cInodeNumber
            cInodeHandler = iOpen(cInodeNumber);
            iSetAccess(cInodeHandler,(uint16_t)mode);
            

            soAddDirEntry(pInodeHandler,bn,cInodeNumber);    //adicionar o direntry, no parent inode, correspondente ao directorio q se está a criar
            iIncRefcount(cInodeHandler);                     //incrementar o refcount no inode, correspondente a direntry do parent Inode 
            iIncRefcount(cInodeHandler); 
            iSetAccess(cInodeHandler,(uint16_t)mode);

            soAddDirEntry(cInodeHandler, ".", cInodeNumber);
            iIncRefcount(pInodeHandler);
                                //incrementar o refcount no inode, correspondente a direntry "." 
            soAddDirEntry(cInodeHandler, "..", pInodeNumber);
            
               
        }
        else{
            throw SOException(ENOTDIR, "Parent inode is not a directory");
        }
        iSave(cInodeHandler);
        iSave(pInodeHandler);
        iClose(cInodeHandler);
        iClose(pInodeHandler);
        return 0;

    }
    catch(SOException & err)
    {
        return -err.en;
    }
}
