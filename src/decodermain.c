#include <stdio.h>
#include <stdlib.h>
#include "decoder.h"


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
    //according to the amd64 manual, the total instruction length
    //can not be longer than 15 bytes
    uint8_t instruction[MAX_SIZE] = { 0 };
    if ( argc == 1 ) {
        argc = loadBinaryStdin(instruction) + 2;
    } else if ( !strHex2Bin(argc, argv, instruction) ) {
        return EXIT_FAILURE;
    }
    decode(argc - 2, instruction);
    return EXIT_SUCCESS;
}
