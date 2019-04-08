IDIR =headers
CC=gcc
CFLAGS=-I $(IDIR) -g -Wextra -Wall -pedantic
#.o files dir
ODIR=src/obj
#.c files dir
CDIR=src

_DEPS = decoder.h cfg.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))


_OBJ = decoder.o decodermain.o cfg.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@ 

decode: $(ODIR)/decoder.o $(ODIR)/decodermain.o
	$(CC) -o $@ $^ $(CFLAGS)

cfg: $(ODIR)/decoder.o $(ODIR)/cfg.o
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm decode
