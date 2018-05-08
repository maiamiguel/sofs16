/*
 * \author Nelson Costa
 * \tester Daniel Alves
 */


#include "mksofs.h"

#include "superblock.h"
#include "exception.h"
#include "inode.h"
#include "direntry.h"
#include "core.h"

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <errno.h>

/*
 * filling in the inode table:
 *   only inode 0 is in use (it describes the root directory)
 */
void fillInInodeTable(SOSuperBlock * p_sb)
{
	unsigned int i, j, k, numInode = 2;
	SOInode it[IPB];


	/*percorre os blocos da tabela de inodes*/
    for(i = 1; i <= p_sb->itsize; i++)
	{
		/*percorre os inodes de cada bloco e preenche os seus campos*/
		for(j = 0; j < IPB; j++)
		{
			for(k = 0; k < N_DIRECT; k++)
				it[j].d[k] = NULL_REFERENCE;

			for(k = 0; k < N_INDIRECT; k++)
				it[j].i1[k] = NULL_REFERENCE;

			it[j].i2 = NULL_REFERENCE;

			/*caso: primeiro inode (diretorio raiz) da tabela de inodes*/
			if (i == 1 && j == 0)
			{
				it[j].mode = S_IFDIR | 0775;
				it[j].refcount = 2;
				it[j].owner = getuid();
				it[j].group = getgid();
				it[j].size = 2 * sizeof(SODirEntry);
				it[j].csize = 1;

				it[j].atime = it[j].mtime = it[j].ctime = time(NULL);

				it[j].d[0] = 0;
			}
			else
			{
				/*caso: último inode da tabela de inodes*/
				if(i == p_sb->itsize && j == (IPB-1))
					it[j].next = NULL_REFERENCE;
				/*restantes inodes da tabela de inodes*/
				else
					it[j].next = numInode++;

				it[j].mode = INODE_FREE;
				it[j].refcount = 0;
				it[j].owner = 0;
				it[j].group = 0;
				it[j].size = 0;
				it[j].csize = 0;
			}
		}
		/*escreve o bloco de inodes no disco*/
		soWriteRawBlock(i, &it);
	}

	//throw SOException(ENOSYS, __FUNCTION__);
}
