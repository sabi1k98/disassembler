#ifndef SYMTAB
#define SYMTAB
#include <stdint.h>


typedef struct {
    uint32_t binding;
    const char* name;
} Symbol;

typedef struct {
    Symbol* content;
    int size;
    int capacity;
} SymbolTable;

SymbolTable parseFunctionNames(char* mapping);

void initSymTab(SymbolTable* symtab,  int size);


#endif
