#include "symtab.h"
#include "elfutils.h"


void initSymTab(SymbolTable* symtab, int size) {
    symtab->content = malloc(sizeof(Symbol) * size);
    if ( !symtab->content ) {
        perror("Symtab malloc\n");
        return;
    }
    symtab->capacity = size;
    symtab->size = 0;
}


int symbolComparator(const void* fst, const void* snd) {
    const char* fststr = ((Symbol*) fst)->name;
    const char* sndstr = ((Symbol*) snd)->name;
    while ( *fststr == '_' ) { fststr++; }
    while ( *sndstr == '_' ) { sndstr++; }
    return strcasecmp(fststr, sndstr);
}

SymbolTable parseFunctionNames(char* mapping) {
    SymbolTable result;

    Elf64_Shdr* symtabHeader = findSection(mapping, ".symtab"); 
    Elf64_Shdr* strtabHeader = findSection(mapping, ".strtab"); 
    if ( !symtabHeader ) {
        fprintf(stderr, "Binary doesn't containt symbol table.\n");
        free(result.content);
        result.content = NULL;
        return result;
    }

    initSymTab(&result, symtabHeader->sh_size / sizeof(Elf64_Sym));
    if ( !result.content ) {
        return result;
    }
    Elf64_Sym* current = (void*) (mapping + symtabHeader->sh_offset);
    for ( size_t i = 0; i < symtabHeader->sh_size / sizeof(Elf64_Sym); i++ ) {
        if ( current->st_name <= 0 || ELF64_ST_TYPE(current->st_info) != STT_FUNC ||
                ELF64_ST_BIND(current->st_info) != STB_GLOBAL ) {
            current++;
            continue;
        }
        result.content[result.size].name = mapping + strtabHeader->sh_offset + current->st_name;
        result.content[result.size].binding = current->st_value;
        result.size++;
        current++;
    }
    qsort(result.content, result.size, sizeof(Symbol), symbolComparator);
    return result;
}

