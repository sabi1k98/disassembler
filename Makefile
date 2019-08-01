IDIR =headers
CC=gcc
CFLAGS=-I $(IDIR) -g -Wextra -Wall -pedantic
#.o files dir
ODIR=src/obj
#.c files dir
CDIR=src
TESTDIR=tests

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

.PHONY: all tests clean

all: decode cfg tests cfg.elf decode.elf

tests: test_hw1 test_hw2


test_hw2: $(ODIR)/test_hw2.o $(ODIR)/decoder.o $(ODIR)/cfg.o
	$(CC) -o $@ $^ $(CFLAGS)

test_hw1: $(ODIR)/test_hw1.o $(ODIR)/decoder.o $(ODIR)/cfg.o
	$(CC) -o $@ $^ $(CFLAGS)

$(ODIR)/maind.o: $(CDIR)/main.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@ -D DECODE

$(ODIR)/maincfg.o: $(CDIR)/main.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@ -D CFG

$(ODIR)/maincfgelf.o: $(CDIR)/main.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@ -D CFG -D ELF

$(ODIR)/maindelf.o: $(CDIR)/main.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@ -D DECODE -D ELF


$(ODIR)/cfgelf.o: $(CDIR)/cfg.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@ -D DECODE -D ELF

$(ODIR)/test%.o: $(TESTDIR)/test%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@ 

$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@ 

decode: $(ODIR)/decoder.o $(ODIR)/maind.o
	$(CC) -o $@ $^ $(CFLAGS)

cfg: $(ODIR)/decoder.o $(ODIR)/cfg.o $(ODIR)/maincfg.o
	$(CC) -o $@ $^ $(CFLAGS)

decode.elf: $(ODIR)/decoder.o $(ODIR)/elfutils.o $(ODIR)/maindelf.o
	$(CC) -o $@ $^ $(CFLAGS)

cfg.elf: $(ODIR)/decoder.o $(ODIR)/cfgelf.o $(ODIR)/elfutils.o $(ODIR)/maincfgelf.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm decode
	rm test_hw1
	rm test_hw2
	rm cfg
	rm decode.elf
	rm cfg.elf
