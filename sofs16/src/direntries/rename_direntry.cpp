/**
 *  \author Vitor Morais
 *  \tester Vitor Morais
 */

#include "direntries.h"

#include "probing.h"
#include "exception.h"
#include <czdealer.h>
#include <filecluster.h>
#include <itdealer.h>
#include <errno.h>

void soRenameDirEntry(int pih, const char *name, const char *newName)
{
    soProbe(500, "soRenameDirEntry(%d, %s, %s)\n", pih, name, newName);

    //throw SOException(ENOSYS, __FUNCTION__);
    SOInode* inode;
    bool entryFound = false;
    inode = iGetPointer(pih);
    uint32_t dpc = soGetDPC();
   	SODirEntry buf [dpc];

    uint32_t dirRpi= inode->size / sizeof(SODirEntry);

    uint32_t last_cluster_idx = dirRpi / dpc; 
	
	for (uint32_t i = 0; i <= last_cluster_idx; i++)
    {
        soReadFileCluster(pih,i,buf);
   
    
        for (uint32_t j = 0; j < dpc; j++)
        {
            if (strcmp(buf[j].name,name) == 0)
            {
            entryFound = true;
            strncpy(buf[j].name, newName, SOFS16_MAX_NAME+1);
            break;
            }
		}
		if(entryFound)
		{
			soWriteFileCluster(pih,last_cluster_idx,buf);
			break;
		}

	}
	if(!entryFound)
	{
		throw SOException(ENOENT, __FUNCTION__);
	
}
}
