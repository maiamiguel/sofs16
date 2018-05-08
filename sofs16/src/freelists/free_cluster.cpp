/**
 *  \author Miguel Maia 76434
 *  \tester Miguel Maia 76434
 */

#include "freelists.h"
#include "probing.h"
#include "exception.h"
#include <errno.h>
#include <core.h>
#include <inttypes.h>
#include "superblock.h"
#include "sbdealer.h"
#include "exception.h"

/*
 * Dictates to be obeyed by the implementation:
 * - parameter cn must be validated, 
 *      throwing a proper error if necessary
 */
void soFreeCluster(uint32_t cn){
    soProbe(732, "soFreeCluster (%u)\n", cn);    

    SOSuperBlock* p_sb;
    p_sb = sbGetPointer();					                                // Acessar o superbloco
    
	if (cn<=0 || cn>p_sb->ctotal-1){                                        // Ver se o cluster dado esta dentro da gama, ou seja, se é um valor válido
		throw SOException(EINVAL, __FUNCTION__);							// return throw (ver erro - man errno)
	}

	//verificar se está cheio -> (in diferente de NULL_REFERENCE)
	if (p_sb->ctail.cache.ref[p_sb->ctail.cache.in] != NULL_REFERENCE){		// Caso a cache esteja cheia, ou seja, a posição in está preenchida, 
		soDeplete();														// é necessário chamar a função deplete para depletar a tail cache														
	}	
    
    p_sb->ctail.cache.ref[p_sb->ctail.cache.in] = cn;						// O cluster passado como argumento passsará para a posição in da tail cache, ou seja, será o mais recente libertado.
    p_sb->ctail.cache.in=(p_sb->ctail.cache.in+1)%FCT_CACHE_SIZE;			// A posição in é incrementada, ficando assim desocupada para ser ocupada pelo próximo cluster libertado.
    p_sb->cfree += 1;                                                       // incrementar o número de clusters livres

    sbSave();																// Guardar as alterações
}
