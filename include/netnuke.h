/**
 *  NetNuke - Erases all storage media detected by the system
 *  Copyright (C) 2009-2010  Joseph Hunkeler <jhunkeler@gmail.com, jhunk@stsci.edu>
 *
 *  This file is part of NetNuke.
 *
 *  NetNuke is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  NetNuke is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NetNuke.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef NETNUKE_H
#define NETNUKE_H

#define AUTHOR          "Joseph Hunkeler"
#define LICENSE         "GPLv3"
#define VERSION_MAJOR   2
#define VERSION_MINOR   0

#define BUS_SCSI    0x02
#define BUS_IDE     0x04
#define BUS_BOTH    (BUS_SCSI | BUS_IDE)
#define MAXLINE     512
#define MAXTHREAD   255

#define NNLOGFILE   "netnuke.log"
#define self        __FUNCTION__

#define _FILE_OFFSET_BITS 64

typedef struct nndevice_t
{
    char vendor[50];
    char model[50];
    char path[50];
    unsigned int blksz;
    unsigned long long blks;
    unsigned long long sz;
} nndevice_t;

void ignore_device(char** list, nndevice_t** d);
int strind(const char* str, const char ch);
int nnlogcleanup();
int COM(const char* func, char *format, ...);
void* wipe(void* device);
pthread_t nnthread(nndevice_t* device);
int nnwrite(int fd, int bsize);
void nnrandinit();
void nnrandfree();
unsigned int nngetseed();
unsigned int nnrand(int min, int max);
char* randstr(int size);
int scanbus_sysfs(nndevice_t** device);
int scanbus(nndevice_t** device,int mask);
void showbus(int mask);
int selectbus(char** flags);
void usage(const char* progname);


#endif //NETNUKE_H
