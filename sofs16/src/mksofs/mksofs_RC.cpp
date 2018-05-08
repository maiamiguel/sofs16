//author: Armando Sousa

#include "mksofs.h"
#include "cluster.h"
#include "superblock.h"
#include "exception.h"

#include <stdio.h>
#include <errno.h>

  /*
   * reset free clusters
   */
void resetFreeCluster(SOSuperBlock * p_sb)
{
	uint32_t array [RPB*p_sb->csize];
	uint32_t bl = p_sb->czstart + ((1 + p_sb->crefs ) * p_sb->csize); 
	// inicio dos clusters + 2blocos (cluster raiz) +  nºblocos da lista de clusters free
	
	for (uint32_t i = 0; i < RPB*p_sb->csize; i++)
	{
		array[i] = 0;
	}	
	
	do
	{
		soWriteRawCluster(bl,&array,p_sb->csize);
		bl = bl +p_sb->csize;
	} while (bl<p_sb->ntotal);
    //throw SOException(ENOSYS, __FUNCTION__);
}
