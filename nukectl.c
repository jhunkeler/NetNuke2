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
#include "netnuke.h"

FILE* randfp;
unsigned int randseed;
unsigned long long total_written_bytes;
extern pthread_mutex_t lock_global;
extern pthread_mutex_t lock_write;

void* wipe(void* device)
{
    srand(nngetseed());
    char buf = '\0';
    nndevice_t* d = (nndevice_t*)device;
    FILE* fp;
    FILE* fptmp = fopen(d->path, "w+t");
    if(fptmp == NULL)
    {
        COM(self, "Unable to open %s: %s\n", d->path, strerror(errno));
        return (int*)1;
    }
    fseek(fptmp, d->sz, SEEK_CUR);
    fwrite(&buf, sizeof(char), 1, fptmp);
    fclose(fptmp);

    fp = NULL;
    fp = fopen(d->path, "w+t");
    if(fp == NULL)
    {
        COM(self, "Unable to open %s: %s\n", d->path, strerror(errno));
        return (int*)1;
    }
    COM(self, "path: %s, block size %d, blocks %lu, total bytes %lu\n",
           d->path, d->blksz, d->blks, d->sz);

    int output_progress = 0;
    unsigned int blocks = d->blksz * d->blks;
    unsigned long long blocks_written = 0;
    while(blocks_written < blocks)
    {
        pthread_mutex_lock(&lock_global);
        blocks_written += nnwrite(fp, d->blksz);
        pthread_mutex_unlock(&lock_global);


        if(output_progress >= 102400)
        {
            long double percent = (blocks_written / (long double)d->sz) * 100;
            printf("%s: %llu of %llu (%0.2Lf%%)\n", d->path, blocks_written, d->sz, percent);
            output_progress = 0;
        }

        ++output_progress;
    }
    COM(self, "%s complete\n", d->path);
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

int nnwrite(FILE* fp, int bsize)
{
    int bytes_written = 0;
    char* buffer = randstr(bsize);
    pthread_mutex_lock(&lock_write);
    //bytes_written = fwrite(buffer, sizeof(char), bsize, fp);
    bytes_written += bsize; //temporary testing
    total_written_bytes += bytes_written;
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
        COM(self, "(urandom) Seed is %lu\n", randseed);
        return randseed;
    }

    unsigned int t = time(NULL);
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
