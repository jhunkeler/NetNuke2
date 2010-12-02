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
#include <getopt.h>
#include <libgen.h> /* for basename() */
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <glob.h>
#include <pthread.h>
#include "netnuke.h"

pthread_mutex_t lock_global = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_write = PTHREAD_MUTEX_INITIALIZER;
extern unsigned long total_written_bytes;
unsigned int blksz_override = 0;
int safety_flag = 0;
int logging_flag = 0;
int verbose_flag = 0;
int bus_mask = 0;
int device_timeout = 0;
char** bus_flags = NULL;

static struct option long_options[] =
{
    {"help",    no_argument,   0,   0},
    {"verbose",    no_argument,   &verbose_flag,   1},
    {"quiet",   no_argument,    &verbose_flag,  0},
    {"safety-off",  no_argument,    &safety_flag,  0},
    {"ignore-first",  no_argument,   0,  'i'},
    {"logging", no_argument,    &logging_flag,  1},
    {"timeout",   required_argument,    0,  't'},
    {"scheme",  required_argument,  0,   's'},
    {"device-type",    required_argument,    0, 'd'},
    {"block-size",  required_argument,  0,   'b'},
    {NULL, 0, 0, 0}
};

static char* long_options_help[] =
{
    "\tThis message",
    "More output",
    "Suppress output",
    "Enable destructive write mode",
    "Ignore first device",
    "Log all output to netnuke.log",
    "Set timeout-to-failure (in seconds)",
    "Set nuking scheme:\n\
                        0 = zero\n\
                        1 = static pattern\n\
                        2 = pure random",
    "Select bus:\n\
                        ide\n\
                        scsi",
    "Block size in bytes",
    NULL
};

char *scsi_device_glob[] =
{
    "/dev/sd*[!0-9]",
    "/dev/nsr*",
    "/dev/tape*",
    NULL
};

char *ide_device_glob[] =
{
    "/dev/hd*[!0-9]",
    NULL
};


void usage(const char* progname)
{
    int i;
    printf("%s v%d.%d %s %s\n", progname, VERSION_MAJOR, VERSION_MINOR, LICENSE, AUTHOR);
    printf("usage: %s ", progname);
    for( i = 0 ; long_options[i].name != NULL ; i++ )
    {
        if(!long_options[i].has_arg)
        {
            if(strncasecmp("help", long_options[i].name, strlen(long_options[i].name)) != 0)
                printf("[--%s] ", long_options[i].name);
        }

        if(long_options[i].has_arg)
            printf("[-%c val] ", (char)long_options[i].val);
    }
    putchar('\n');

    for( i = 0 ; long_options[i].name != NULL ; i++)
    {
        printf(" --%s\t", long_options[i].name);
        if(long_options[i].has_arg)
        {
            printf("-%c\t", (char)long_options[i].val);
        }
        else
        {
            printf("\t");
        }
        printf("%s\n", long_options_help[i]);
    }
    exit(0);
}

int main(int argc, char* argv[])
{
    uid_t uid=getuid(), euid=geteuid();
    if (uid < 0 || uid != euid)
    {
        COM(self, "Need root... exiting\n");
        exit(1);
    }

    if((nnlogcleanup()) != 0)
    {
        fprintf(stderr, "Failed to cleanup %s: %s\n", NNLOGFILE, strerror(errno));
    }
    COM(self, "Program start\n");

    if(argc < 2) usage(basename(argv[0]));
    int c;
    int option_index = 0;

    while((c = getopt_long (argc, argv, "ib:s:d:t:", long_options, &option_index)) > 0)
    {
        switch (c)
        {
            case 0:
            {
                if (long_options[option_index].flag != 0) break;
                printf ("option %s", long_options[option_index].name);
                if (optarg) printf (" with arg %s", optarg);
                printf ("\n");
                break;
            }
            case 'b':
            {
                blksz_override = atoi(optarg);
                if(blksz_override < 512)
                {
                    COM(self, "Block size must be a multiple of 512\n");
                    exit(1);
                }
                COM(self, "Forcing %d block size\n", blksz_override);
                break;
            }
            case 's':
            {
                break;
            }
            case 't':
            {
                device_timeout = atoi(optarg);
                COM(self, "%ds timeout set\n", device_timeout);
                break;
            }
            case 'd':
            {
                int i = 0;
                char* tok = strtok(optarg, ",");
                bus_flags = (char**)malloc(1024);
                while(tok !=NULL)
                {
                    bus_flags[i] = (char*)malloc(strlen(tok));
                    strncpy(bus_flags[i], tok, strlen(tok));
                    i++;
                    tok = strtok(NULL, ",");
                }
                break;
            }
            case 'h':
            case ':':
            case '?':
                usage(basename(argv[0]));
                break;
        }
    }

    if (optind < argc)
    {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
    }

    COM(self, "Safety is %s\n", safety_flag ? "ON" : "OFF");

    nndevice_t** device;
    device = (nndevice_t**)malloc(MAXTHREAD * sizeof(nndevice_t));
    if(device == NULL)
    {
        perror("device list");
        exit(1);
    }
    pthread_t thread[MAXTHREAD];
    int thread_count = 0;
    int i;

    bus_mask = selectbus(bus_flags);
    scanbus(device, bus_mask);
    nnrandinit();

    COM(self, "Initializing mutex\n");
    pthread_mutex_init(&lock_global, NULL);

    COM(self, "Generating threads\n");
    for( i = 0 ; device[i] != NULL ; i++ )
    {
        thread[i] = (pthread_t)nnthread(device[i]);
        COM(self, "thread id: %ld\n", thread[i]);
    }

    thread_count = i;
    COM(self, "Joining %d thread%c\n", thread_count, (thread_count > 1 || thread_count < 1) ? 's' : '\b');
    for( i = 0 ; i < thread_count ; i++ )
    {
        pthread_join(thread[i], NULL);
    }

    COM(self, "Destroying mutex\n");
    pthread_mutex_destroy(&lock_global);

    COM(self, "Total bytes written: %lu\n", total_written_bytes);
    nnrandfree();
    return 0;
}
