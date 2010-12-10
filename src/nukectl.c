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
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include "netnuke.h"

FILE* randfp;
int writing = 0;
unsigned int randseed;
unsigned long long total_written_bytes = 0;
extern unsigned int blksz_override;
extern int verbose_flag;
extern int safety_flag;
extern pthread_mutex_t lock_global;
extern pthread_mutex_t lock_write;

void ignore_device(char** list, nndevice_t** d)
{
    int i = 0;
    int j = 0;

    for( i = 0 ; d[i] != NULL ; i++ )
    {
        for( j = 0 ; list[j] != NULL ; j++ )
        {
            if(!strncasecmp(d[i]->path, list[j], strlen(list[j])))
            {
                COM(self, "%s\n", list[j]);
                memset(d[i], 0, sizeof(nndevice_t));
                //memmove(d[end-1], d[i], sizeof(nndevice_t));
            }
        }
    }
}

void* wipe(void* device)
{
    extern WINDOW* main_window, *info_window;
    extern pthread_mutex_t main_window_lock;
    nndevice_t* d = (nndevice_t*)device;
    unsigned long long times = 0;
    unsigned long long block = 0;
    unsigned long long bytes_out = 0;
    long double percent = 0.0L;

    if(d->path[0] == 0)
    {
        return NULL;
    }
    if(blksz_override > 0)
    {
        d->blksz = blksz_override;
    }

    int fd = open(d->path, O_WRONLY | O_SYNC);
    if(fd < 0)
    {
        COM(self, "Unable to open %s: %s\n", d->path, strerror(errno));
        return (int*)1;
    }
    COM(self, "%s, block size %d, blocks %llu, total bytes %llu\n", d->path, d->blksz, d->blks, d->sz);
    srand(nngetseed());

    times = d->sz / d->blksz;
    while(block <= times)
    {
        if(!writing)
        {
            if(verbose_flag)
            {
                percent = (long double)((bytes_out / (long double)d->sz) * 100);
                //COM(self, "%s: %llu of %llu (%0.2Lf%%)\n", d->path, bytes_out, d->sz, percent);
                mvwprintw(info_window, 0, 0, "%s: %llu of %llu (%0.2Lf%%)\n", d->path, bytes_out, d->sz, percent);
                wrefresh(info_window);
            }
            bytes_out += nnwrite(fd, d->blksz);
            block++;
        }
    }
    close(fd);
    COM(self, "%s complete, wrote %llu bytes\n", d->path, bytes_out);
    pthread_exit(NULL);

    return NULL;
}

pthread_t nnthread(nndevice_t* device)
{
    pthread_t thread;
    if((pthread_create(&thread, NULL, wipe, device)) != 0)
    {
        COM(self, "Failed to create thread %lu\n", thread);
        return 1;
    }

    return thread;
}

int nnwrite(int fd, int bsize)
{
    unsigned int bytes_written = 0;
    char* buffer = randstr(bsize);
    pthread_mutex_lock(&lock_write);
    writing = 1;
    if(safety_flag)
    {
        /* simulation */
        bytes_written += bsize;
    }
    else
    {
        /* destructive */
        //bytes_written = write(fd, buffer, bsize);
        bytes_written += bsize;
    }
    writing = 0;
    //total_written_bytes += bytes_written;
    pthread_mutex_unlock(&lock_write);

    free(buffer);
    return bytes_written;
}

void nnrandinit()
{
    COM(self, "Initializing random seed\n");
    randfp = fopen("/dev/urandom", "r");
    if(randfp == NULL)
    {
        COM(self, "urandom: %s\n", strerror(errno));
        return;
    }
    srand(nngetseed());
}

void nnrandfree()
{
    COM(self, "Closing urandom\n");
    fclose(randfp);
}

unsigned int nngetseed()
{
    if((fread(&randseed, 1, sizeof(int), randfp)) > 0)
    {
        if(verbose_flag)
            COM(self, "(urandom) Seed is %lu\n", randseed);
        return randseed;
    }

    unsigned int t = time(NULL);
    if(verbose_flag)
        COM(self, "(UNIX Epoch) Seed is %lu\n", t);
    return t;
}

unsigned int nnrand(int min, int max)
{
    return rand()%(min + max);
}

char* randstr(int size)
{
    char* buffer = (char*)malloc(sizeof(char) * size + 1);
    unsigned char c;
    int i;
    for( i = 0 ; i < size ; i++ )
    {
         c = nnrand(1, 256);
         buffer[i] = c;
    }

    return buffer;
}
