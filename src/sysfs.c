
/* sysfs.c
   Helper functions to write to the sysfs files that control the GPIO and PWM.
   
   Copyright 2018 Antonio Serrano Hernandez

   This file is part of rfsgpio.

   rfsgpio is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   rfsgpio is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with rfsgpio; see the file COPYING.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#include <errno.h>      // errno
#include <fcntl.h>      // open
#include <string.h>     // strlen
#include <unistd.h>     // write

/* Write a value to a sysfs (actually any) file.

   The file is opened, value written and then closed again.

   Parameters:
     * file: name of the file to write to.
     * val: value to write to the file.
*/
int
write_sysfs_file(const char *file, const char *val)
{
    int fd, e;
    size_t len;

    fd = open(file, O_WRONLY);
    if (fd < 0) {
        return -1;
    }
    len = strlen(val);
    if (write(fd, val, len) != len) {
        // Save errno to avoid polute it with the close syscall
        e = errno;
        close(fd);
        errno = e;
        return -1;
    }
    close(fd);
    return 0;
}

/* Read a value from a sysfs (actually any) file.

   The file is opened, readed (up to n - 1 bytes) and closed. The read value is
   copied into val buffer.

   Parameters:
     * file: name of the file to read from.
     * val: buffer where to store the read value. The user must ensure that
         this buffer has enough capacity.
     * n: capacity that val must have.
*/
int
read_sysfs_file(const char *file, char *val, size_t n)
{
    int fd, e, r;

    fd = open(file, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    if ((r = read(fd, val, n - 1)) < 0) {
        e = errno;
        close(fd);
        errno = e;
        return -1;
    }
    val[r] = '\0';
    close(fd);
}

