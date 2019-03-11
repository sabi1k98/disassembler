#include "decoder.h"


bool isREXprefix(const uint8_t byte) {
    return (byte & 0xF0) == 0x40;
}

bool isEscaped(const uint8_t instruction) {
    return instruction == SECONDARY_ESCAPE;
}


uint8_t get8BitValue(instructionData* data) {
    return data->instruction[data->index];
}


uint32_t get32BitValue(instructionData* data) {
    data->instruction += data->index;
    uint32_t ret = *((uint32_t*) data->instruction);
    data->instruction -= data->index;
    return ret;
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

char* regValue2String(int regValue, char* string) {
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


char* findGPR(instructionData* data, char* string) {
    int regValue;
    regValue = data->opcode | (data->rex.b << 3);
    return regValue2String(regValue, string);
}


char* decodeMoveInstruction(instructionData* data, ModRM modrm, char* string) { 
    if ( modrm.rm != 0x5) {
        return NULL;
    }
    int regNum = modrm.reg | (data->rex.r << 3); 
    char source[20] = { 0 };
    char destination[10] = { 0 };
    data->index++;
    if (modrm.mod == 0x00) {
        sprintf(source, "(%%rip)+0x%x", get32BitValue(data));
    } else {
        sprintf(source, "(%%rbp)+0x%x", get8BitValue(data));
    }
    if ( data->opcode == 0x8B ) {
        strcat(string, source);
        strcat(string, ", ");
        strcat(string, regValue2String(regNum, destination));
    } else {
        strcat(string, regValue2String(regNum, destination));
        strcat(string, ", ");
        strcat(string, source);
    }
    return string;
}

char* findRegisters(instructionData* data, char* string) {
    uint8_t modRMByte = data->instruction[data->index];
    ModRM modrm = { modRMByte >> 6, modRMByte >> 3, modRMByte};
    if ( (data->opcode == MOV_MEM_REG || data->opcode == MOV_REG_MEM) && modrm.mod != 0x03 ) {
       return decodeMoveInstruction(data, modrm, string); 
    } else if ( modrm.mod != 0x03 || (data->opcode == MUL && modrm.reg != 0x4) ||
        data->rex.lowerNibble != 0x4 || !data->rex.w ) {
        return NULL;
    }
    int firstReg = modrm.reg | (data->rex.r << 3); 
    int secondReg = modrm.rm | (data->rex.b << 3); 
    if ( data->opcode != MUL ) {
        regValue2String(firstReg, string);
        strcat(string, ", ");
    }
    return regValue2String(secondReg, string);
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
        sprintf(string, "%s", "cmp\t"); 
        return true;
    } else if ( data->opcode == MOV_REG_MEM || data->opcode == MOV_MEM_REG ) {
        sprintf(string, "%s", "mov\t"); 
        return true;
    } else if ( data->opcode == MUL ) {
        sprintf(string, "%s", "mul\t"); 
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


bool getExpectedParams(instructionData* data, int remaining) {
        if ( data->opcode == JMP_REL8OFF || data->opcode == JE_REL8OFF || data->opcode == JNE_REL8OFF ||
                data->opcode == JB_REL8OFF ) {
            if ( remaining == 1) {
                data->expectedParams[0] = DISPLACEMENT_8;
                return true;
            }
            return false;
        }

        if ( data->opcode == JMP_REL32OFF || data->opcode == JE_REL32OFF || data->opcode == JNE_REL32OFF ||
                data->opcode == JB_REL32OFF || data->opcode == CALL_REL32OFF ) {
            if ( remaining == 4 ) {
                data->expectedParams[0] = DISPLACEMENT_32;
                return true;
            }
            return false;
        }

        if ( data->opcode == RET || data->opcode == INT3 || data->opcode == NOP ) {
            return true;
        }

        if ( data->opcode == PUSH_IMM64 || data->opcode == XOR_REG_IMM32 ||
                data->opcode == ADD_REG_IMM32 || data->opcode == CMP_REG_IMM32 ) {
            if ( remaining == 4 ) {
                data->expectedParams[0] = IMM64;
                return true;
            }
            return false;
        }


        if ( (data->opcode >= PUSH_REG64 && data->opcode <= PUSH_REG64 + 0x07)
                || (data->opcode >= POP_REG64 && data->opcode <= POP_REG64 + 0x07) ) {
            if ( remaining == 0 ) {
                data->expectedParams[0] = RQ;
                return true;
            }
            return false;
        }

        if ( data->opcode == CMP_REG_MEM || data->opcode == CMP_MEM_REG ||
                data->opcode == MUL ) {
            if ( remaining == 1 ) {
                data->expectedParams[0] = MODrm;
                return true;
            }
            return false;
        }

        if ( data->opcode == MOV_REG_MEM || data->opcode == MOV_MEM_REG ) {
            data->expectedParams[0] = MODrm;
            return true;
        }
    return false;
}



void printInstruction(instructionData* data) {
    char result[60] = { 0 };
    char buffer[20] = { 0 };
    if ( !findOpcode(data, result) ) {
        fprintf(stderr, "Unknown instruction\n");
        return;
    }
    int i = 0;
    while( data->expectedParams[i] != NONE ) {
        switch ( data->expectedParams[i] ) {
            case DISPLACEMENT_8:
                sprintf(buffer, "(%%rip)+0x%x", get8BitValue(data));
                data->index++;
                strcat(result, buffer);
                break;
            case DISPLACEMENT_32:
                sprintf(buffer, "(%%rip)+0x%x", get32BitValue(data));
                data->index += 4;
                strcat(result, buffer);
                break;
            case IMM64:
                sprintf(buffer, "$0x%x", get32BitValue(data));
                data->index += 4;
                strcat(result, buffer);
                break;
            case RQ:
                strcat(result, findGPR(data, buffer));
                data->index++;
                break;
            case MODrm:
                if ( !findRegisters(data, result) ) {
                    printf("Unknown instruction: register fuckup\n");
                    return;
                }
                data->index++;
                break;
        }
        i++;
    }
    printf("%s\n", result);
}



void decode(int length, uint8_t* instruction) {
    instructionData data = {0, { 0 }, false, 0, NULL, instruction};
    if ( isREXprefix(instruction[data.index]) ) {
        data.rex.lowerNibble = instruction[data.index] >> 4;
        data.rex.w = instruction[data.index] >> 3;
        data.rex.r = instruction[data.index] >> 2;
        data.rex.x = instruction[data.index] >> 1;
        data.rex.b = instruction[data.index];
        data.index++;
    }
    if ( isEscaped(instruction[data.index]) ) {
        data.isEscaped = true;
        data.index++;
    }
    data.opcode = instruction[data.index];
    data.index++;
    params expectedParams[4] = { NONE };
    data.expectedParams = expectedParams;
    if ( !getExpectedParams(&data, length - data.index) ) {
        fprintf(stderr, "Unknown instruction: Param fuckup\n");
        return;
    }
    printInstruction(&data);
}
