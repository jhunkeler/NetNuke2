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
#include "netnuke.h"

extern int verbose_flag;

int nnlogcleanup()
{
    int status = 0;
    if((access(NNLOGFILE, W_OK)) == 0)
    {
        status = unlink(NNLOGFILE);
    }

    return status;
}

int COM(const char* func, char *format, ...)
{
    struct tm *logtm;
    time_t logtime = time(NULL);
    char timestr[64];
    char *str = (char*)malloc(sizeof(char) * 256);
    char tmpstr[255];
    int n;

    /*FILE *logfp = fopen(NNLOGFILE, "a+");
    if(logfp == NULL)
    {
        COM(self, "Unable to open %s\n", NNLOGFILE);
        verbose_flag = 1;
    }*/
    va_list args;
    va_start (args, format);
    n = vsprintf (str, format, args);
    va_end (args);

    logtm = localtime(&logtime);
    snprintf(timestr, sizeof(timestr), "%02d-%02d-%02d %02d:%02d:%02d", logtm->tm_year+1900, logtm->tm_mon+1, logtm->tm_mday, logtm->tm_hour, logtm->tm_min, logtm->tm_sec);
    snprintf(tmpstr, sizeof(tmpstr), "%s _%s_: %s", timestr, func, str);

    if(verbose_flag)
    {
        fprintf(stdout, "%s", tmpstr);
        //fprintf(logfp, "%s", tmpstr);
    }
    else
    {
        /*fprintf(logfp, "%s", tmpstr);*/
    }

    free(str);
    /*fclose(logfp);*/

    return 0;
}
