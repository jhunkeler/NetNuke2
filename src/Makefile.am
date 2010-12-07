AUTOMAKE_OPTIONS = foreign

AM_CFLAGS=-Wall -O2 -I../include -I../sysfsutils/include -static
AM_LDFLAGS=-lpthread -lrt

sbin_PROGRAMS= netnuke

netnuke_SOURCES=	bus.c		\
			misc.c		\
			netnuke.c	\
			nukectl.c	\
			output_redirect.c
