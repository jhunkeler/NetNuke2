AUTOMAKE_OPTIONS = foreign

AM_CFLAGS=-Wall -W -Wstrict-prototypes -I../include -static
AM_LDFLAGS=-L../libsysfs -lpthread -lsysfs -all-static

sbin_PROGRAMS= netnuke

netnuke_SOURCES=	bus.c		\
			misc.c		\
			netnuke.c	\
			nukectl.c	\
			output_redirect.c
