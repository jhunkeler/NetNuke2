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
#include <errno.h>
#include <dirent.h>
#include "netnuke.h"

extern char* scsi_device_glob[];
extern char* ide_device_glob[];

int scanbus_sysfs(nndevice_t** device)
{
    const char* scsi_path = "/sys/class/scsi_disk";
    char tmpstr[128];
    int i = 0;
    int fd = -1;
    FILE* fp = NULL;
    DIR* pbase;
    DIR* pblock;
    struct dirent* pbaseent;
    struct dirent* pblockent;

    /* scan for scsi devices (and with libata enabled ... ide devices too) via sysfs */
    if((access(scsi_path, F_OK)) != 0)
    {
        COM(self, "Sysfs not available\n");
        return 1;
    }

    pbase = opendir(scsi_path);
    if(!pbase)
    {
        COM(self, "fatal opendir: %s\n", strerror(errno));
        exit(1);
    }

    while((pbaseent = readdir(pbase)))
    {
        device[i] = (nndevice_t*)malloc(sizeof(nndevice_t));
        if(device[i] == NULL)
        {
            perror("device");
            exit(1);
        }

        if(!strncasecmp(pbaseent->d_name, ".", 1) || !strncasecmp(pbaseent->d_name, "..", 2))
            continue;

        /* set model */
        snprintf(tmpstr, sizeof(tmpstr), "%s/%s/device/model", scsi_path, pbaseent->d_name);
        fp = fopen(tmpstr, "r");
        if(fp == NULL)
        {
            COM(self, "Could not retrieve model information from %s: %s\n", tmpstr, strerror(errno));
        }
        else
        {
            if(fgets(device[i]->model, sizeof(device[i]->model), fp) != NULL)
                device[i]->model[strlen(device[i]->model) - 1] = 0;

            fclose(fp);
        }

        /* set vendor */
        snprintf(tmpstr, sizeof(tmpstr), "%s/%s/device/vendor", scsi_path, pbaseent->d_name);
        fp = fopen(tmpstr, "r");
        if(fp == NULL)
        {
            COM(self, "Could not retrieve vendor information from %s: %s\n", tmpstr, strerror(errno));
        }
        else
        {
             /* Why does sysfs not terminate the string after the last character?  Kernel 2.6 bug?
             Here we are checking for the space character and terminating it manually. */
            if((fgets(device[i]->vendor, sizeof(device[i]->model), fp)) != NULL)
                device[i]->vendor[strind(device[i]->vendor, ' ')] = 0;

            fclose(fp);
        }

        snprintf(tmpstr, sizeof(tmpstr), "%s/%s/device/block", scsi_path, pbaseent->d_name);
        pblock = opendir(tmpstr);
        if(!pblock)
        {
            COM(self, "fatal opendir: %s\n", strerror(errno));
            exit(1);
        }

        while((pblockent = readdir(pblock)))
        {
            if(!strncasecmp(pblockent->d_name, ".", 1) || !strncasecmp(pblockent->d_name, "..", 2))
                continue;

            /* set path */
            snprintf(device[i]->path, sizeof(device[i]->path), "/dev/%s", pblockent->d_name);
        }
        closedir(pblock);

        /* set number of blocks */
        fd = open(device[i]->path, O_RDONLY | O_NONBLOCK);
        if(fd >= 0)
        {
            if((ioctl(fd, BLKGETSIZE, &device[i]->blks)) != 0)
            {
                COM(self, "ioctl: could not retrieve block count for %s\n", device[i]->path);
                device[i]->blks = 0;
            }
            close(fd);
        }

        device[i]->blksz = 512;
        device[i]->sz = device[i]->blks * 512;

        COM(self, "%s %s %s %llu\n", device[i]->vendor, device[i]->path, device[i]->model, (device[i]->blks * device[i]->blksz));
        ++i;
    }
    closedir(pbase);

    return 0;
}

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

    while(flags[i] != NULL)
    {
        if(!strcmp(flags[i], "ide"))
        {
            mask |= BUS_IDE;
            COM(self, "IDE (0x%02X)\n", mask);
        }

        if(!strcmp(flags[i], "scsi"))
        {
            mask |= BUS_SCSI;
            COM(self, "SCSI (0x%02X)\n", mask);
        }

        i++;
    }

    return mask;
}

