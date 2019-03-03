#include <stdio.h>
#include "decoder.h"





int main(int argc, const char* argv[]) {
    //according to the amd64 manual, the total instruction length
    //can not be longer than 15 bytes
    if ( argc > MAX_BYTES ) { 
        return 1;
    }
    uint8_t instruction[MAX_BYTES] = { 0 };
    if ( !strHex2Bin(argc, argv, instruction) ) {
        return 1;
    }
    for ( int i=0 ; i < 15 ; i++ ) {
        printf("%x", instruction[i]);
    }
    putchar('\n');
    return 0;
}
