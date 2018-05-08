/*
 * \author ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
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

#include "syscalls.h"

#include "probing.h"
#include "exception.h"

/*
 *  \brief Make a new link to a file.
 *
 *  It tries to emulate <em>link</em> system call.
 *
 *  \param path path to an existing file
 *  \param newPath new path to the same file
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soLink(const char *path, const char *newPath)
{
    soProbe(225, "soLink(\"%s\", \"%s\")\n", path, newPath);

    try
    {
        /* substitute this throw by your code */
        return -ENOSYS;
    }
    catch(SOException & err)
    {
        return -err.en;
    }

}
