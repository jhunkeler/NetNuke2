CFLAGS=-Wall -O2 -s
LDFLAGS=-lpthread
OBJS= \
      src/bus.c \
      src/netnuke.c \
      src/nukectl.c \
      src/output_redirect.c

all:
	cc ${CFLAGS} ${LDFLAGS} -o netnuke ${OBJS}
	du -sh netnuke
clean:
	rm -rf netnuke
