/*
 *  Abel Neto
 *  37623    
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
 *  \brief Delete a link to a file from a directory and possibly the file it refers to from the file system.
 *
 *  It tries to emulate <em>unlink</em> system call.
 *
 *  \param path path to the file to be deleted
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soUnlink(const char *path)
{
    soProbe(226, "soUnlink(\"%s\")\n", path);

    try
    {
        char* xpath = strdupa (path);           // duplica o array de chars "path"
        char* bn = strdupa(basename(xpath));    // nome da direntry a ser criada
        char* dn = dirname(xpath);              // nome/path do diretorio onde se quer criar o novo diretorio

        uint32_t inD;                           // InodeNumber of the Directory!
        uint32_t inL;                           // InodeNumber of the link!    
        uint32_t cinp; 

        soTraversePath(dn, &inD);   
        uint32_t ihD = iOpen(inD);
        soGetDirEntry(ihD, bn, &inL);
       
        if(inL == NULL_REFERENCE)  {                      //Verificar o caminho devolve um inode com fich!//            
            throw SOException(ENOENT, __FUNCTION__);    
        }
        
        uint32_t ihL = iOpen(inL);            
        SOInode* inodeLink;
        inodeLink = iGetPointer(ihL);
        if((inodeLink->mode & S_IFREG) == S_IFREG){           //VERIFICAMOS SE O INODE É DO TIPO FILEREG!
            
            uint32_t refcount = iDecRefcount(ihL);
            soDeleteDirEntry(ihD, bn, &cinp);

            if (refcount == 0){
                soFreeInode(inL);
            } 
        }
        if((inodeLink->mode & S_IFLNK) == S_IFLNK){           //VERIFICAMOS SE O INODE É DO TIPO SYMLINK!
            soFreeInode(inL);
            soDeleteDirEntry(ihD, bn, &cinp);
        }
        iSave(inL);
        iClose(inL);
        iSave(ihD);
        iClose(ihD);

        return 0;   

    }
    catch(SOException & err)
    {
        return -err.en;
    }
}
