CFLAGS=-Wall -O2 
LDFLAGS=-lpthread
objects=netnuke.o nukectl.o bus.o output_redirect.o

netnuke: ${objects}
	${CC} ${CFLAGS} ${LDFLAGS} -o netnuke ${objects}
	
netnuke.o: netnuke.h
nukectl.o: netnuke.h
bus.o: netnuke.h
output_redirect.o: netnuke.h

.PHONY: clean
clean:
	-rm netnuke ${objects}
