#include "decoder.h"


bool strHex2Bin(int argc, const char** argv, uint8_t* result) {
    for ( int i=0 ; i < argc ; i++ ) {
        if ( sscanf(argv[i], "%2hhX", &result[i]) != 1 ) {
            return 1;
        }
    }
    return 0;
}


bool isREXprefix(const uint8_t byte) {
    return (byte ^ 0x0400) >= 0x00FF;
}

char* opcodeToString(uint8_t opcode) {
    if (opcode == JMP_REL8OFF || opcode == JMP_REL32OFF) {
        return "JMP";
    }
    return NULL;
}


void getExpectedParams(uint8_t opcode, int remaining, params* params) {
    switch(opcode) {
        case JMP_REL8OFF:
            if ( remaining == 1) {
                params[0] = DISPLACEMENT_8;
            } else {
                params[0] = ERROR;
            }
            break;
        case JMP_REL32OFF:
            if (remaining == 2) {
                params[0] = DISPLACEMENT_16;
            } else if (remaining == 4) {
                params[0] = DISPLACEMENT_32;
            } else {
                params[0] = ERROR;
            }
            break;
    }
}


int8_t get8BitDisplacement(uint8_t* instruction, int start) {
    return instruction[start];
}


int16_t get16BitDisplacement(uint8_t* instruction, int start) {
    return (int16_t) instruction[start];
}


int32_t get32BitDisplacement(uint8_t* instruction, int start) {
    return (int32_t) instruction[start];
}


char* printInstruction(uint8_t opcode, params* expectedParams, uint8_t* instruction, int start) {
    char result[4] = opcodeToString(opcode);
}

char* decode(int length, uint8_t* instruction) {
    int i = 0;
    if ( isREXprefix(instruction[0]) ) {
        i++;
    }
    uint8_t opcode = instruction[i];
    i++;
    params expectedParams[7] = { NONE };
    getExpectedParams(opcode, length - i, expectedParams);
}
