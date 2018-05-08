//author: Vitor Morais
#include "mksofs.h"

#include "superblock.h"
#include "exception.h"

#include "direntry.h"

#include <errno.h>

/*
 * Root Directory:
         first 2 entries filled in with "." and ".." references
         the other entries are filled with '\n'
 */
void fillInRootDir(SOSuperBlock * sbp)
{
    uint32_t dpc = DPB * sbp->csize;

    // Initializing directories array
    SODirEntry dir_array[dpc];

    // first position of the array named '.' and its reference must be 0
    strncpy(dir_array[0].name, ".", SOFS16_MAX_NAME+1);
    dir_array[0].in = 0;

    // second position of the array named '..' and its reference must be equal to his parent node reference
    strncpy(dir_array[1].name, "..", SOFS16_MAX_NAME+1);
    dir_array[1].in = 0;

    // remaining entries must be empty
    for(size_t i = 2; i < dpc; i++) {
        strncpy(dir_array[i].name, "\0", SOFS16_MAX_NAME+1);
        dir_array[i].in = NULL_BLOCK;
    }

    soWriteRawCluster(sbp->czstart, &dir_array, sbp->csize); //writing in the first cluster
}
