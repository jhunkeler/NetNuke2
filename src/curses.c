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
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include "netnuke.h"

WINDOW* main_window;
WINDOW* info_window;
pthread_mutex_t main_window_lock = PTHREAD_MUTEX_INITIALIZER;

WINDOW *create_window(int height, int width, int starty, int startx)
{
    WINDOW *window;
    window = newwin(height, width, starty, startx);
    wborder(window, '+', '+', '+','+','+','+','+','+');
    wrefresh(window);

    return window;
}

void free_window(WINDOW* window)
{
    wborder(window, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(window);
    delwin(window);
}

void update_window(WINDOW* window)
{
    wborder(window, '+', '+', '+','+','+','+','+','+');
    wrefresh(window);
}

extern pthread_mutex_t lock_global;
extern pthread_mutex_t lock_write;
extern unsigned long long total_written_bytes;
extern unsigned int blksz_override;
extern int list_flag;
extern int ignore_first_flag;
extern int ignore_flag;
extern int safety_flag;
extern int logging_flag;
extern int verbose_flag;
extern int bus_mask;
extern int device_timeout;
extern char** bus_flags;
extern char** ignore_list;
extern nndevice_t** device;

void* main_window_worker(void* args)
{
    pthread_t thread[MAXTHREAD];
    int thread_count = 0;
    int i = 0;
    device = (nndevice_t**)malloc(MAXTHREAD * sizeof(nndevice_t));
    if(device == NULL)
    {
        perror("device list");
        exit(1);
    }


    if(list_flag)
    {
        bus_mask = selectbus(bus_flags);
        scanbus_sysfs(device);
        exit(0);
    }

    /* Select the bus mask and scan for devices */
    bus_mask = selectbus(bus_flags);
    scanbus_sysfs(device);

    /* If the operator wants to preserve the first device */
    if(ignore_first_flag)
    {
        int first = 0;
        int last = 0;

        /* Count how many devices we have */

        int i = 0;
        while(device[i] != NULL)
        {
            i++;
        }
        last = i - 1;

        COM("IGNORING: %s", device[first]->path);
        /* Replace the first device's array entry and then clear the original */
        memmove(device[first], device[last], sizeof(nndevice_t));
        memset(device[last], 0, sizeof(nndevice_t));
        device[last] = NULL;
    }

    if(ignore_flag)
    {
        ignore_device(ignore_list, device);
    }

    /* Run check to see if any devices were returned */
    if(device[0] == NULL)
    {
        COM(self, "No devices detected\n");
        exit(0);
    }

    COM(self, "Safety is %s", safety_flag ? "OFF" : "ON");
    if(device_timeout) COM(self, "%ds timeout set\n", device_timeout);
    if(blksz_override) COM(self, "Forcing %d block size\n", blksz_override);

    COM(self, "Initializing mutex\n");
    pthread_mutex_init(&lock_global, NULL);
    pthread_mutex_init(&lock_write, NULL);
    COM(self, "Generating threads\n");

    /* Tell the random generator to start */
    nnrandinit();

    /* Start a single thread per device node*/
    for( i = 0; device[i] != NULL ; i++ )
    {
        thread[i] = (pthread_t)nnthread(device[i]);
        COM(self, "thread id: %8X %8X\n", thread[i]);
    }
    /* Catch up */
    usleep(10000);

    /* Using the original device count, set thread_count and join all threads */
    thread_count = i;
    COM(self, "Joining %d thread%c\n", thread_count, (thread_count > 1 || thread_count < 1) ? 's' : '\b');

    for( i = 0 ; i < thread_count ; i++)
    {
        pthread_join(thread[i], NULL);
    }

    COM(self, "Destroying mutex\n");
    pthread_mutex_destroy(&lock_global);
    pthread_mutex_destroy(&lock_write);
    COM(self, "Total bytes written: %lu\n", total_written_bytes);

    nnrandfree();
    return NULL;
}

void main_init()
{
    pthread_t main_window_thread;
    pthread_t info_window_thread;

	int startx = 0, starty = 0, width = 0, height = 0;

    cbreak();
	keypad(stdscr, TRUE);

	height = getmaxy(stdscr) / 2;
	width = getmaxx(stdscr);

    main_window = create_window(height, width, starty, startx);
    starty = getmaxy(stdscr) / 2;
    height = getmaxy(stdscr) / 2;
    width = getmaxx(stdscr);
    info_window = create_window(height, width, starty, startx);
    //refresh();

    pthread_mutex_init(&main_window_lock, NULL);
    pthread_create(&main_window_thread, NULL, main_window_worker, NULL);
    pthread_join(main_window_thread, NULL);
    pthread_mutex_destroy(&main_window_lock);
}

void main_deinit()
{
    endwin();
}
