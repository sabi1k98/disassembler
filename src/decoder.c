#include "decoder.h"


bool strHex2Bin(int argc, const char** argv, uint8_t* result) {
    for ( int i=0 ; i < argc ; i++ ) {
        if ( sscanf(argv[i], "%2hhX", &result[i]) != 1 ) {
            return 1;
        }
    }
    return 0;
}
