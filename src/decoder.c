#include "decoder.h"


bool isREXprefix(const uint8_t byte) {
    return (byte ^ 0x040) >= 0x00FF;
}

bool isEscaped(const uint8_t instruction) {
    return instruction == SECONDARY_ESCAPE;
}

bool findSecondaryOpcode(const uint8_t opcode, char* string) {
    if ( opcode == JNE_REL32OFF ) {
        sprintf(string, "%s\t", "JNE");
        return true; 
    } else if ( opcode == JB_REL32OFF ) { 
        sprintf(string, "%s\t", "JNE");
        return true; 
    }
    return false;
}

bool findOpcode(const uint8_t opcode, bool isEscaped, char* string) {
    if ( isEscaped ) {
        return findSecondaryOpcode(opcode, string);
    }
    //printf("%x\n", opcode);
    if ( opcode == JMP_REL8OFF || opcode == JMP_REL32OFF ) {
        sprintf(string, "%s\t", "JMP");
        return true;
    } else if ( opcode == JE_REL8OFF || opcode == JE_REL32OFF ) {
        sprintf(string, "%s\t", "JE"); 
        return true;
    } else if ( opcode == JNE_REL8OFF ) {
        sprintf(string, "%s\t", "JNE"); 
        return true;
    } else if ( opcode == JB_REL8OFF ) {
        sprintf(string, "%s\t", "JB"); 
        return true;
    }
    return false;
}


bool getExpectedParams(uint8_t opcode, int remaining, params* params) {
        if ( opcode == JMP_REL8OFF || opcode == JE_REL8OFF || opcode == JNE_REL8OFF ||
                opcode == JB_REL8OFF ) {
            if ( remaining == 1) {
                params[0] = DISPLACEMENT_8;
                return true;
            }
            return false;
        }
        if ( opcode == JMP_REL32OFF || opcode == JE_REL32OFF || opcode == JNE_REL32OFF ||
                opcode == JB_REL32OFF ) {
            if ( remaining == 4 ) {
                params[0] = DISPLACEMENT_32;
                return true;
            }
            return false;
        }
    return false;
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


void printInstruction(instructionData* data, uint8_t* instruction) {
    char result[50] = { 0 };
    char buffer[20];
    if ( !findOpcode(data->opcode, data->isEscaped, result) ) {
        fprintf(stderr, "Unknown instruction\n");
        return;
    }
    int i = 0;
    while( data->expectedParams[i] != NONE ) {
        switch ( data->expectedParams[i] ) {
            case DISPLACEMENT_8:
                sprintf(buffer, "(%%rip)+0x%x", get8BitValue(instruction, data->index));
                data->index++;
                strcat(result, buffer);
                break;
            case DISPLACEMENT_32:
                sprintf(buffer, "(%%rip)+0x%x", get32BitValue(instruction, data->index));
                data->index += 4;
                strcat(result, buffer);
                break;
        }
        i++;
    }
    printf("%s\n", result);
}



void decode(int length, uint8_t* instruction) {
    instructionData data = {0, { 0 }, false, 0, NULL};
    if ( isREXprefix(instruction[data.index]) ) {
        data.rex.lowerNibble = (0xf0 | instruction[data.index]) >> 4;
        data.rex.w = 0x8 | instruction[data.index];
        data.rex.r = 0x4 | instruction[data.index];
        data.rex.x = 0x2 | instruction[data.index];
        data.rex.b = 0x1 | instruction[data.index];
        data.index++;
    }
    if ( isEscaped(instruction[data.index]) ) {
        data.isEscaped = true;
        data.index++;
    }
    data.opcode = instruction[data.index];
    data.index++;
    params expectedParams[3] = { NONE };
    data.expectedParams = expectedParams;
    if ( !getExpectedParams(data.opcode, length - data.index, expectedParams) ) {
        printf("%x\n", data.opcode);
        printf("I fucked up here\n");
        fprintf(stderr, "Unknown instruction\n");
        return;
    }
    printInstruction(&data, instruction);
}
