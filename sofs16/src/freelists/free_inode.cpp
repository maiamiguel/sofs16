/**
 *  \author Daniel Alves
 *  \tester Nelson Costa
 */

#include "freelists.h"

#include "probing.h"
#include "exception.h"
#include <core.h>
#include <errno.h>
#include <inttypes.h>
#include "superblock.h"
#include "sbdealer.h"
#include "inode.h"
#include "sbdealer.h"
#include "itdealer.h"
/*
 * Dictates to be obeyed by the implementation:
 * - parameter in must be validated, 
 *      throwing a proper error if necessary
 */
void soFreeInode(uint32_t in)
{
    soProbe(712, "soFreeInode (%u)\n", in);

    //throw SOException(ENOSYS, __FUNCTION__);

    SOSuperBlock* p_sb;
    SOInode* p_inode, * p_itail; 
    int Inode, Itail;
    int i;
	
	p_sb = sbGetPointer();

	//Verifica se o indice do in é válido
    if(in>(p_sb->itotal-1) || (in<1)) {         
    	sbSave();
		throw SOException(EINVAL, __FUNCTION__);
	}


	//Abertura do Handler do inode
	Inode =iOpen(in);

	//Carregar ponteiro pro inode
	p_inode = iGetPointer(Inode);


  	//Verificar que se inode está livre
    if( p_inode->mode == INODE_FREE ){
    	iClose(Inode);
    	sbSave();
    	throw SOException(EINVAL, __FUNCTION__);
  	}

	//Verificar se possui referencias
    if( (p_inode->refcount !=2)){
    	iClose(Inode);
    	sbSave();
    	throw SOException(EINVAL, __FUNCTION__);
  	}
  	//Preenchimento dos campos do inode 
  	p_inode->next = NULL_REFERENCE;
  	p_inode->mode = p_inode->mode | INODE_FREE;
	p_inode->refcount = 0;
	p_inode->owner = 0;
	p_inode->group = 0;
	p_inode->size = 0;
	p_inode->csize = 0;

	for(i = 0; i < N_DIRECT; i++)
		p_inode->d[i] = NULL_REFERENCE;

	for(i = 0; i < N_INDIRECT; i++)
		p_inode->i1[i] = NULL_REFERENCE;

	p_inode->i2 = NULL_REFERENCE;

	//Guardar inode
	iSave(Inode);

	//Fechar inode
	iClose(Inode);

	if(p_sb->ifree == 0)
	{
		//Se a lista estiver vazia
		p_sb->ihead = p_sb->itail = in;
	}
	else
	{
		//Abrir inode tail
		Itail =iOpen(p_sb->itail);

		//Carregar o inode no fim da lista
		p_itail = iGetPointer(Itail);

		//Colocar o inode libertado a seguir ao último inode da lista de inodes lives
		p_itail->next = in;					

		//Guardar inode
		iSave(Itail);

		//Fechar inode
		iClose(Itail);
		p_sb->itail = in;
	}
	
	//Atualizar contagem do número de inode livres
	p_sb->ifree +=1;
	sbSave();
}
