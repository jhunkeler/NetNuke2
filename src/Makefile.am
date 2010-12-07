AUTOMAKE_OPTIONS = foreign

AM_CFLAGS=-Wall -O2 -I../include -static
AM_LDFLAGS=-lpthread -lrt -lsysfs

sbin_PROGRAMS= netnuke

netnuke_SOURCES=	bus.c		\
			misc.c		\
			netnuke.c	\
			nukectl.c	\
			output_redirect.c
