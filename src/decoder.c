#include "decoder.h"


bool isREXprefix(const uint8_t byte) {
    return (byte ^ 0x040) >= 0x00FF;
}

bool opcodeToString(const uint8_t opcode, char* string) {
    //printf("%x\n", opcode);
    if (opcode == JMP_REL8OFF || opcode == JMP_REL32OFF) {
        sprintf(string, "%s\t", "JMP");
        return true;
    }
    return false;
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
            if ( remaining == 4 ) {
                params[0] = DISPLACEMENT_32;
            } else {
                params[0] = ERROR;
            }
            break;
    }
}


uint8_t get8BitValue(const uint8_t* instruction, int start) {
    return instruction[start];
}


uint32_t get32BitValue(const uint8_t* instruction, int start) {
    instruction += start;
    uint32_t ret = *((uint32_t*) instruction);
    instruction -= start;
    return ret;
}


char* printInstruction(uint8_t opcode, params* expectedParams, uint8_t* instruction, int start) {
    char result[50] = { 0 };
    char buffer[20];
    opcodeToString(opcode, result);
    int i = 0;
    while( expectedParams[i] != NONE ) {
        switch ( expectedParams[i] ) {
            case DISPLACEMENT_8:
                sprintf(buffer, "(%%rip)+%x", get8BitValue(instruction, start));
                start++;
                strcat(result, buffer);
                break;
            case DISPLACEMENT_32:
                sprintf(buffer, "(%%rip)+%x", get32BitValue(instruction, start));
                start += 4;
                strcat(result, buffer);
                break;
        }
        i++;
    }
    printf("%s\n", result);
    return NULL;
}

void decode(int length, uint8_t* instruction) {
    int i = 0;
    if ( isREXprefix(instruction[0]) ) {
        i++;
    }
    uint8_t opcode = instruction[i];
    i++;
    params expectedParams[3] = { NONE };
    getExpectedParams(opcode, length - i, expectedParams);
    printInstruction(opcode, expectedParams, instruction, i);
}
