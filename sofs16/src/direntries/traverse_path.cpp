/**
 *  \author Nelson Costa
 *  \tester Daniel Alves
 */

#include "direntries.h"
#include <itdealer.h>
#include <core.h>
#include "probing.h"
#include "exception.h"

#include <errno.h>
#include <libgen.h>

void soTraversePath(char *path, uint32_t * inp)
{
    soProbe(400, "soTraversePath(%s, %p)\n", path, inp);

    //throw SOException(ENOSYS, __FUNCTION__);

    if(path[0] != '/')
    	throw SOException(EINVAL,__FUNCTION__); //throw Invalid argument

    uint32_t path_length = strlen(path);
    if(path_length == 1 && path[0] == '/')
    {
        *inp = 0;
        return;
    }

    char *xpath = strdupa(path);
    char *bname = strdupa(basename(xpath));
    char *dname = dirname(xpath);

    uint32_t cinp;

    if(strcmp(dname,"/") == 0)
    {
    	int ih = iOpen(0);
        if(!iCheckAccess(ih, 01)) /*2º argumento é as permissões do user,
                                    neste caso, verifica se o user tem permissão de execução*/
            throw SOException(EACCES,__FUNCTION__); //throw Permission denied
    	soGetDirEntry(ih, bname, &cinp);
    	if(cinp == NULL_REFERENCE)
            throw SOException(ENOENT,__FUNCTION__); //throw No such file or directory
        *inp = cinp;
        iClose(ih);
        return;
    }

    soTraversePath(dname, inp);
    int ih = iOpen(*inp);
    if(!iCheckAccess(ih, 01)) /*2º argumento é as permissões do user,
                                neste caso, verifica se o user tem permissão de execução*/
        throw SOException(EACCES,__FUNCTION__); //throw Permission denied
    soGetDirEntry(ih, bname, &cinp);
    if(cinp == NULL_REFERENCE)
        throw SOException(ENOENT,__FUNCTION__); //throw No such file or directory
    *inp = cinp;
    iClose(ih);
    return;
}
