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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include "netnuke.h"

extern char* scsi_device_glob[];
extern char* ide_device_glob[];

int scanbus(nndevice_t** device, int mask)
{
    int fd = -1;
    int i = 0;
    unsigned int j = 0;
    int blocksize = 512;
    unsigned long blocks = 0;
    unsigned long size = 0;


    if(mask & BUS_SCSI)
    {
        for( i = 0 ; scsi_device_glob[i] != NULL ; i++ )
        {
            glob_t entries;
            if(glob(scsi_device_glob[i], 0, NULL, &entries) == 0)
            {
                for( j = 0 ; j < entries.gl_pathc ; j++ )
                {
                    fd = open(entries.gl_pathv[j], O_RDONLY | O_NONBLOCK);
                    if(fd >= 0)
                    {
                        if((ioctl(fd, BLKGETSIZE, &blocks)) == 0)
                        {
                            device[j] = (nndevice_t*)malloc(sizeof(nndevice_t));
                            if(device[j] == NULL)
                            {
                                perror("device");
                                exit(1);
                            }

                            size = blocks * blocksize;
                            strncpy(device[j]->path, entries.gl_pathv[j], sizeof(device[j]->path));
                            device[j]->blks = blocks;
                            device[j]->sz = size;
                            device[j]->blksz = 512;

                            printf("%s ", device[j]->path);
                            printf("%llu %llu %.2f\n", device[j]->blks, device[j]->sz, (double)device[j]->sz / (1024 * 1024 * 1024));

                        }
                        close(fd);
                    }
                }
                globfree(&entries);
            }
        }

    }

    if(mask & BUS_IDE)
    {
        for( i = 0 ; ide_device_glob[i] != NULL ; i++ )
        {
            glob_t entries;
            if(glob(ide_device_glob[i], 0, NULL, &entries) == 0)
            {
                for( j = 0 ; j < entries.gl_pathc ; j++ )
                {
                    fd = open(entries.gl_pathv[j], O_RDONLY | O_NONBLOCK);
                    if(fd >= 0)
                    {

                        if((ioctl(fd, BLKGETSIZE, &blocks)) == 0)
                        {
                            size = blocks * blocksize;
                            printf("%s ", entries.gl_pathv[j]);
                            printf("%lu %lu %.2f\n", blocks, size, (double)size / (1024 * 1024 * 1024));
                        }
                        close(fd);
                    }
                }
                globfree(&entries);
            }
        }
    }

    return 0;
}

int selectbus(char** flags)
{
    int mask = 0;
    int i = 0;

    if(flags == NULL)
    {
        return (mask = BUS_BOTH);
    }

    mask = 0;
    while(flags[i] != NULL)
    {
        if(!strcmp(flags[i], "ide"))
        {
            mask |= BUS_IDE;
            printf("IDE (0x%02X)\n", mask);
        }

        if(!strcmp(flags[i], "scsi"))
        {
            mask |= BUS_SCSI;
            printf("SCSI (0x%02X)\n", mask);
        }

        i++;
    }

    return mask;
}

