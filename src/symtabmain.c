#include <symtab.h>
#include <stdlib.h>
#include <stdio.h>
#include <elfutils.h>

void printSymtab(SymbolTable* table) {
    for ( int i = 0; i < table->size; i++ ) {
        printf("%016x T %s\n", table->content[i].binding, table->content[i].name);
    }
}


int main(int argc, char* argv[]) {
    if ( argc != 2 ) {
        fprintf(stderr, "No input file provided\n");
        return EXIT_FAILURE;
    }
    int size;
    char* mapping = mapElf(argv[1], &size);
    if ( !mapping ) {
        return EXIT_FAILURE;
    }
    SymbolTable symtab = parseFunctionNames(mapping); 
    printSymtab(&symtab);
    munmap(mapping, size);
    free(symtab.content);
    return EXIT_SUCCESS; 
}
