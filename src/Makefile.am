AUTOMAKE_OPTIONS = foreign

AM_CFLAGS= -Wall -O2 -I$(top_srcdir)/include 
netnuke_LDFLAGS= -lpthread

sbin_PROGRAMS = netnuke

netnuke_SOURCES =	bus.c		\
			misc.c		\
			netnuke.c	\
			nukectl.c	\
			output_redirect.c
