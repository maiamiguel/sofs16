/*
 * \author ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
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
 *  \brief Creates a symbolic link which contains the given path.
 *
 *  It tries to emulate <em>symlink</em> system call.
 *
 *  \remark The permissions set for the symbolic link should have read (r), write (w) and execution (x) permissions for
 *          both <em>user</em>, <em>group</em> and <em>other</em>.
 *
 *  \param effPath path to be stored in the symbolic link file
 *  \param path path to the symbolic link to be created
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soSymlink(const char *effPath, const char *path)
{
    soProbe(235, "soSymlink(\"%s\", \"%s\")\n", effPath, path);

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
