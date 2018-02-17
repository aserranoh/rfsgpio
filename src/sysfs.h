
/* sysfs.h
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

#ifndef SYSFS_H
#define SYSFS_H

#include <sys/types.h>  // size_t

#define RFS_MAX_SYSFS_STR    16

/* Write a value to a sysfs (actually any) file.

   The file is opened, value written and then closed again.

   Parameters:
     * file: name of the file to write to.
     * val: value to write to the file.
*/
int
write_sysfs_file(const char *file, const char *val);

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
read_sysfs_file(const char *file, char *val, size_t n);

#endif

