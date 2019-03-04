IDIR =headers
CC=clang
CFLAGS=-I $(IDIR) -g -Wextra -Wall -pedantic

ODIR=src/obj

CDIR=src

_DEPS = decoder.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))


_OBJ = decoder.o decodermain.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@ 

decode: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm decode
