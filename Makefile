CFLAGS=-Wall -O2 -s
LDFLAGS=-lpthread
OBJS= \
      bus.c \
      netnuke.c \
      nukectl.c \
      output_redirect.c

all:
	cc ${CFLAGS} ${LDFLAGS} -o netnuke ${OBJS}
	du -sh netnuke
clean:
	rm -rf netnuke
