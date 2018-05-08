/**
 *  \Abel Neto
 *  \Abel Neto
 */
#include "core.h"
#include "superblock.h"
#include "direntries.h"
#include "probing.h"
#include "exception.h"
#include <czdealer.h>
#include <filecluster.h>
#include <itdealer.h>
#include <errno.h>

void soGetDirEntry(int pih, const char *name, uint32_t * cinp)
{
    soProbe(500, "soGetDirEntry(%d, %s, %p)\n", pih, name, cinp);

    SOInode* inode;
    bool entryFound = false;
    inode = iGetPointer(pih);
    uint32_t dpc = soGetDPC();
   	SODirEntry buf [dpc];

    uint32_t dirRpi= inode->size / sizeof(SODirEntry);		//nº de entradas associadas ao INODE
    uint32_t last_cluster_idx = dirRpi / dpc;				//last_cluster_id = Quociente de (sizeOfInode/DPC) ->size of Inode corresponde ao nºentradas ut

	
	for (uint32_t i = 0; i <= last_cluster_idx; i++)
    {
        soReadFileCluster(pih,i,buf);
        for (uint32_t j = 0; j < dpc; j++)
        {
            if (strcmp(buf[j].name,name) == 0)
            {
            entryFound = true;
            *cinp = buf[j].in;
            }
		}
		if(entryFound)
		{
			break;//break outer cycle
		}
		//check in another cluster
	}
	if(!entryFound)
	{
	   * cinp = NULL_REFERENCE;
	}
}
    