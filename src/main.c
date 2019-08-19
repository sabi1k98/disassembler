#include <stdio.h>
#include <stdlib.h>

#ifdef DECODE
#include "decoder.h"
#endif

#ifdef CFG
#include "cfg.h"
#endif

#ifdef ELF
#include "elfutils.h"
#endif


bool strHex2Bin(int argc, const char** argv, uint8_t* result) {
    for ( int i = 1; i < argc; i++ ) {
        if ( sscanf(argv[i], " %hhx ", &result[i - 1]) != 1 ) {
            return false;
        }
    }
    return true;
}


int loadBinaryStdin(uint8_t* instruction) {
   freopen(NULL, "rb", stdin); 
   return fread(instruction, 1, MAX_SIZE, stdin);
}


int main(int argc, const char* argv[]) {
    uint8_t* instruction;
    int offset = 0;
    #ifdef ELF
    if (argc != 2) {
        fprintf(stderr, "No binary provided\n");
        return EXIT_FAILURE;
    }
    void* mapping = mapElf(argv[1], &argc);
    if ( !mapping ) {
        return EXIT_FAILURE;
    }
    if ( !isValidElf(mapping) ) {
        instruction = mapping;
    } else {
        Elf64_Shdr* section = findSection(mapping, ".text");
        if ( !section ) {
            munmap(mapping, argc);
            return EXIT_FAILURE;
        }
        argc = section->sh_size + 1;
        instruction = (uint8_t*) mapping + section->sh_offset;
        offset = ((Elf64_Ehdr*) mapping)->e_entry;
    }
    #else
    uint8_t instructionArray[MAX_SIZE] = { 0 };
    instruction = instructionArray;
    if ( argc == 1 ) {
        argc = loadBinaryStdin(instruction) + 1;
    } else if ( !strHex2Bin(argc, argv, instruction) ) {
        return EXIT_FAILURE;
    }
    #endif
    #ifdef DECODE
    decodeAll(argc - 1, instruction, offset);
    #endif
    #ifdef CFG
    makeGraph(argc - 1, instruction, offset);
    #endif
    #ifdef ELF
    munmap(mapping, argc);
    #endif
    return EXIT_SUCCESS;
}
