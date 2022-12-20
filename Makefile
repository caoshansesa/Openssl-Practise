IDIR =../include
CC=g++
CFLAGS= -I$(IDIR) -g -O0 -fpermissive

ODIR=.

LIBS=-lncurses -lssl -lcrypto

_DEPS =
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ =https_request.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: https_request fetch_http_crl bio_socket

https_request: https_request.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

fetch_http_crl: http_crl.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

bio_socket: bio_socket.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *~ core $(INCDIR)/*~
	rm -f https_request fetch* bio_socket
	rm -f *.o


