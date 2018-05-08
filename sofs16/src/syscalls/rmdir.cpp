/*
 * \author Armando Sousa
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
#include "filecluster.h"
#include "freelists.h"
#include "syscalls.h"
#include "itdealer.h"
#include "probing.h"
#include "exception.h"
#include "czdealer.h"
#include "core.h"

/*
 *  \brief Delete a directory.
 *
 *  It tries to emulate <em>rmdir</em> system call.
 *  
 *  The directory should be empty, ie. only containing the '.' and '..' entries.
 *
 *  \param path path to the directory to be deleted
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soRmdir(const char *path)
{
    soProbe(233, "soRmdir(\"%s\")\n", path);

    try
    {  
        char* xpath = strdupa(path);            
        char* bn = strdupa(basename(xpath));    // Name of the dir to be erased
        char* dn = dirname(xpath);              // Dir path to the parent dir

        uint32_t pin;
        uint32_t sin;
        soTraversePath(dn, &pin);         
         
        if (pin == NULL_REFERENCE){
            throw SOException(ENOENT, __FUNCTION__);
        }
        
        int pih = iOpen(pin);                   //Get Parent Inode Number        
        
        soGetDirEntry(pih,bn,&sin);
        
        int sih = iOpen(sin);
        SOInode* sInode =iGetPointer(sih);

        if (sInode->refcount-2 != 0)
        {
            throw SOException(ENOTEMPTY,__FUNCTION__);
        }
        
        soDeleteDirEntry(pih,bn,&sin);
        iDecRefcount(pih);
        soFreeInode(sin); 
        iSave(sin);
        iSave(pin);  
        iClose(sin);
        iClose(pin);
        
        return 0;
      
    }
    catch(SOException & err)
    {
        return -err.en;
    }
}