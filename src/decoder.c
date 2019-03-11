#include "decoder.h"


bool isREXprefix(const uint8_t byte) {
    return (byte & 0xF0) == 0x40;
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

char* findGPR(instructionData* data, char* string) {
    int regValue;
    regValue = data->opcode | (data->rex.b << 3) ;
    switch (regValue) {
        case RAX:
            strcat(string, "%rax");
            break;
        case RCX:
            strcat(string, "%rcx");
            break;
        case RDX:
            strcat(string, "%rdx");
            break;
        case RBX:
            strcat(string, "%rbx");
            break;
        case RSP:
            strcat(string, "%rsp");
            break;
        case RBP:
            strcat(string, "%rbp");
            break;
        case RSI:
            strcat(string, "%rsi");
            break;
        case RDI:
            strcat(string, "%rdi");
            break;
        default:{
            char buffer[5] = { 0 };
            sprintf(buffer, "%%r%d", regValue);
            strcat(string, buffer);
            break;
            }
    }
    return string;
}

bool findOpcode(instructionData* data, char* string) {
    if ( data->isEscaped ) {
        return findSecondaryOpcode(data->opcode, string);
    }
    //printf("%x\n", opcode);
    if ( data->opcode == JMP_REL8OFF || data->opcode == JMP_REL32OFF ) {
        sprintf(string, "%s\t", "jmp");
        return true;
    } else if ( data->opcode == JE_REL8OFF || data->opcode == JE_REL32OFF ) {
        sprintf(string, "%s\t", "je"); 
        return true;
    } else if ( data->opcode == JNE_REL8OFF ) {
        sprintf(string, "%s\t", "jne"); 
        return true;
    } else if ( data->opcode == JB_REL8OFF ) {
        sprintf(string, "%s\t", "jb"); 
        return true;
    } else if ( data->opcode == RET ) {
        sprintf(string, "%s", "ret"); 
        return true;
    } else if ( data->opcode == CALL_REL32OFF ) {
        sprintf(string, "%s\t", "call"); 
        return true;
    } else if ( data->opcode == PUSH_IMM64 ||
            (data->opcode >= PUSH_REG64 && data->opcode <= PUSH_REG64 + 0x7) ) {
        sprintf(string, "%s\t", "push"); 
        data->opcode -= PUSH_REG64;
        return true;
    } else if ( data->opcode >= POP_REG64 && data->opcode <= POP_REG64 + 0x7 ) {
        sprintf(string, "%s\t", "pop"); 
        data->opcode -= POP_REG64;
        return true;
    } else if ( data->opcode == INT3 ) {
        sprintf(string, "%s", "int3"); 
        return true;
    } else if ( data->opcode == NOP ) {
        sprintf(string, "%s", "nop"); 
        return true;
    } else if ( data->opcode == CMP_REG_MEM || data->opcode == CMP_MEM_REG ) {
        sprintf(string, "%s", "cmp"); 
        return true;
    } else if ( data->opcode == XOR_REG_IMM32 ) {
        if ( data->rex.w ) {
            sprintf(string, "%s", "xor\t%rax, "); 
        } else {
            sprintf(string, "%s", "xor\t%eax, "); 
        }
        return true;
    } else if ( data->opcode == ADD_REG_IMM32 ) {
        if ( data->rex.w ) {
            sprintf(string, "%s", "add\t%rax, "); 
        } else {
            sprintf(string, "%s", "add\t%eax, "); 
        }
        return true;
    } else if ( data->opcode == CMP_REG_IMM32 ) {
        if ( data->rex.w ) {
            sprintf(string, "%s", "cmp\t%rax, "); 
        } else {
            sprintf(string, "%s", "cmp\t%eax, "); 
        }
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
                opcode == JB_REL32OFF || opcode == CALL_REL32OFF ) {
            if ( remaining == 4 ) {
                params[0] = DISPLACEMENT_32;
                return true;
            }
            return false;
        }

        if ( opcode == RET || opcode == INT3 || opcode == NOP ) {
            return true;
        }

        if ( opcode == PUSH_IMM64 || opcode == XOR_REG_IMM32 ||
                opcode == ADD_REG_IMM32 || opcode == CMP_REG_IMM32 ) {
            if ( remaining == 4 ) {
                params[0] = IMM64;
                return true;
            }
            return false;
        }


        if ( (opcode >= PUSH_REG64 && opcode <= PUSH_REG64 + 0x07)
                || (opcode >= POP_REG64 && opcode <= POP_REG64 + 0x07) ) {
            if ( remaining == 0 ) {
                params[0] = RQ;
                return true;
            }
            return false;
        }

        if ( opcode == CMP_REG_MEM || opcode == CMP_MEM_REG ) {
            if ( remaining == 1 ) {
                params[0] = MODrm;
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
    char buffer[20] = { 0 };
    if ( !findOpcode(data, result) ) {
        printf("%d\n", data->opcode);
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
            case IMM64:
                sprintf(buffer, "$0x%x", get32BitValue(instruction, data->index));
                data->index += 4;
                strcat(result, buffer);
                break;
            case RQ:
                strcat(result, findGPR(data, buffer));
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
        data.rex.w = (0x8 & instruction[data.index]) >> 3;
        data.rex.r = (0x4 & instruction[data.index]) >> 2;
        data.rex.x = (0x2 & instruction[data.index]) >> 1;
        data.rex.b = 0x1 & instruction[data.index];
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
        fprintf(stderr, "Unknown instruction\n");
        return;
    }
    printInstruction(&data, instruction);
}
