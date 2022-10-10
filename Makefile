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

https_request: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *~ core $(INCDIR)/*~
	rm -f https_request
	rm -f *.o

etags:
	find . -type f -iname "*.[ch]" | xargs etags --append
