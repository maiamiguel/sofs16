/**
 *  \author Nelson Costa 42983
 *  \tester Nelson Costa 42983
 */

#include "freelists.h"

#include "probing.h"
#include "exception.h"
#include "superblock.h"
#include "sbdealer.h"
#include "itdealer.h"
#include "core.h"
#include "inode.h"

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <errno.h>
#include <inttypes.h>

/*
 * Dictates to be obeyed by the implementation:
 * - error ENOSPC should be thrown if there is no free inodes
 * - the allocated inode must be properly initialized
 */
void soAllocInode(uint32_t type, uint32_t * inp)
{
    soProbe(711, "soAllocInode(%u, %p)\n", type, inp);

    //throw SOException(ENOSYS, __FUNCTION__);

    SOSuperBlock *sbp;
    SOInode *ip;
    int i, ih;

    /*verifica se o tipo do inode é válido (diretório, ficheiro ou symbolic link)*/
    if( (type != S_IFDIR) && (type != S_IFREG) && (type != S_IFLNK) )
    	throw SOException(EINVAL, __FUNCTION__);

    /*verifica se o ponteiro inp (que aponta para um número de inode) é valido*/
    if( inp == NULL )
    	throw SOException(EINVAL, __FUNCTION__);

    /*carrega o superbloco*/
    sbp = sbGetPointer();

    /*verifica a existência de free inodes*/
    if( sbp->ifree == 0 )
    	throw SOException(ENOSPC, __FUNCTION__);

    /*guarda o número do inode a ser alocado*/
    *inp = sbp->ihead;

    /*obtém um ponteiro para a cabeça da lista de free inodes*/
    ih = iOpen(*inp);
    ip = iGetPointer(ih);

    /*atualiza a cabeça da lista de free inodes no superbloco*/
    sbp->ihead = ip->next;

    /*inicializa o inode*/
    ip->mode = type; 
    ip->refcount = 0;
    ip->owner = getuid();
    ip->group = getgid();
    ip->size = 0;
    ip->csize = 0;
    ip->atime = ip->mtime = ip->ctime = time(NULL);
    
    for(i = 0; i < N_DIRECT; i++)
		ip->d[i] = NULL_REFERENCE;

	for(i = 0; i < N_INDIRECT; i++)
		ip->i1[i] = NULL_REFERENCE;

	ip->i2 = NULL_REFERENCE;

    /*guarda e fecha o inode*/
    iSave(ih);
    iClose(ih);

    /*atualiza o número de free inodes no superbloco*/
    sbp->ifree--;

    /*verifica a existência de free inodes após a alocação de um free inode*/
    /*se não houver free inodes, atualiza a cauda da lista de free inodes no superbloco*/
    if(sbp->ifree == 0)
    	sbp->itail = NULL_REFERENCE;

    /*guarda as alterações feitas no superbloco*/
    sbSave();
 
 }
