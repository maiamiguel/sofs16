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
 *  \brief Read the value of a symbolic link.
 *
 *  It tries to emulate <em>readlink</em> system call.
 *
 *  \param path path to the symbolic link
 *  \param buff pointer to the buffer where data to be read is to be stored
 *  \param size buffer size in bytes
 *
 *  \return 0 on success; 
 *      -errno in case of error, being errno the system error that better represents the cause of failure
 */
int soReadlink(const char *path, char *buff, size_t size)
{
    soProbe(236, "soReadlink(\"%s\", %p, %u)\n", path, buff, size);

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
