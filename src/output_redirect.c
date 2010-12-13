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
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include "netnuke.h"

extern int logging_flag;

int nnlogcleanup()
{
    int status = 0;
    if((access(NNLOGFILE, W_OK)) == 0)
    {
        status = unlink(NNLOGFILE);
    }

    return status;
}

extern WINDOW* main_window;
extern pthread_mutex_t main_window_lock;
int printrow = 0;
int COM(const char* func, char *format, ...)
{
    struct tm *logtm;
    time_t logtime = time(NULL);
    char timestr[64];
    char *str = (char*)malloc(sizeof(char) * 256);
    char tmpstr[255];
    int n;

    FILE *logfp = NULL;
    if(logging_flag)
    {
        logfp = fopen(NNLOGFILE, "a+");
        if(logfp == NULL)
        {
            fprintf(stderr, "Unable to open %s\n", NNLOGFILE);
        }
    }
    va_list args;
    va_start (args, format);
    n = vsprintf (str, format, args);
    va_end (args);

    logtm = localtime(&logtime);

    if(logging_flag)
    {
        snprintf(timestr, sizeof(timestr), "%02d-%02d-%02d %02d:%02d:%02d", logtm->tm_year+1900, logtm->tm_mon+1, logtm->tm_mday, logtm->tm_hour, logtm->tm_min, logtm->tm_sec);
        snprintf(tmpstr, sizeof(tmpstr), "%s _%s_: %s", timestr, func, str);
	/* With ncurses we will need to append a newline character because
           calls to COM don't have them */
        fprintf(logfp, "%s\n", tmpstr);
        fclose(logfp);
	goto screen_out;
    }
    else
    {
screen_out:
        snprintf(tmpstr, sizeof(tmpstr), "_%s_: %s", func, str);
        //fprintf(stdout, "%s", tmpstr);
        pthread_mutex_lock(&main_window_lock);
        //mvprintw(main_window, printrow, 2, "%s", tmpstr);
        wprintw(main_window, "%s", tmpstr);
        wmove(main_window, printrow, 2);
        update_window(main_window);
	if(printrow >= (getmaxy(main_window) / 2))
	    wscrl(main_window, -1);

        printrow++;
        pthread_mutex_unlock(&main_window_lock);
    }

    free(str);
    return 0;
}
