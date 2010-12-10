sbin_PROGRAMS= netnuke

netnuke_CFLAGS=-Wall -W -Wstrict-prototypes -I../include -static
netnuke_LDFLAGS=-L../libsysfs -lncurses -lpthread -lsysfs -all-static

netnuke_SOURCES=	bus.c		\
			misc.c		\
			netnuke.c	\
			nukectl.c	\
			output_redirect.c
