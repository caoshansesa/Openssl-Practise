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

all: http_crl https_request

https_request: https_request.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

https_crl: http_crl.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *~ core $(INCDIR)/*~
	rm -f https_request
	rm -f *.o


