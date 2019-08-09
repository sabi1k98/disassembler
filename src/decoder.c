#include "decoder.h"


bool isREXprefix(const uint8_t byte) {
    return (byte & TOP_4_BYTES) == 0x40;
}

bool isEscaped(const uint8_t instruction) {
    return instruction == SECONDARY_ESCAPE;
}


int8_t get8BitValue(instructionData* data) {
    return data->instruction[data->index++];
}


int32_t get32BitValue(instructionData* data) {
    data->instruction += data->index;
    uint32_t ret = *((uint32_t*) data->instruction);
    data->instruction -= data->index;
    data->index += 4;
    return ret;
}

uint32_t computeAddress(uint32_t current, uint32_t relative) {
    return current + (int32_t)relative;
}


bool findSecondaryOpcode(const uint8_t opcode, char* string) {
    switch ( opcode ) {
        case JNE_REL32OFF:
            sprintf(string, "%s\t", "jne");
            return true;
        case JB_REL32OFF:
            sprintf(string, "%s\t", "jb");
            return true; 
        case JE_REL32OFF:
            sprintf(string, "%s\t", "je");
            return true; 
        case MUL_REG_REG:
            sprintf(string, "%s\t", "imul");
            return true;
        default:
            return false;
    }
}

char* regValue2String(int regValue, char* string) {
    const char* registry[] = { "%rax", "%rcx", "%rdx", "%rbx", "%rsp", "%rbp", "%rsi", "%rdi",
   				"%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15" };
    strcat(string, registry[regValue]);
    return string;
}


char* findGPR(instructionData* data, char* string) {
    int regValue = data->opcode;
    if ( data->opcode >= PUSH_REG64 && data->opcode <= PUSH_REG64 + 0x07 ) {
        regValue -= PUSH_REG64;
    } else if ( data->opcode >= POP_REG64 && data->opcode <= POP_REG64 + 0x07 ) {
        regValue -= POP_REG64;
    } else {
        return NULL; 
    }
    regValue |= (data->rex.b << 3);
    return regValue2String(regValue, string);
}

void sprintSignedHex(int32_t num, char* dest) {
    sprintf(dest, "%s0x%x", num < 0 ? "-" : "", num < 0 ? -num : num);
}

char* decodeMoveInstruction(instructionData* data, ModRM modrm, char* string) { 
    if ( modrm.rm != 0x5) { //ONLY 0b101 as r/m can encode RIP-relative addressing
        return NULL;
    }
    int regNum = modrm.reg | (data->rex.r << 3); 
    char source[20] = { 0 };
    char destination[10] = { 0 };
    data->index++;
    uint32_t offset = get32BitValue(data);
    sprintSignedHex(offset, source);
    if (modrm.mod == 0x00) { //RIP-relative addressing
        strcat(source, "(%rip)");
    } else { //base + offset with RBP as base
        strcat(source, "(%rbp)");
    }
    if ( data->opcode == MOV_REG_MEM ) {
        strcat(string, source);
        strcat(string, ",");
        strcat(string, regValue2String(regNum, destination));
    } else {
        strcat(string, regValue2String(regNum, destination));
        strcat(string, ",");
        strcat(string, source);
    }
    if ( modrm.mod == 0x00 ) {
        sprintf(source, " # 0x%x + ", data->index + data->offset);
        strcat(string, source);
    } else {
        strcat(string, " # (%rbp) + ");
    }
    sprintSignedHex(offset, source);
    strcat(string, source);
    return string;
}

char* findRegisters(instructionData* data, char* string) {
    uint8_t modRMByte = data->instruction[data->index];
    ModRM modrm = { modRMByte >> 6, modRMByte >> 3, modRMByte};
    if ( (data->opcode == MOV_MEM_REG || data->opcode == MOV_REG_MEM) && modrm.mod != 0x03 ) {
       return decodeMoveInstruction(data, modrm, string); 
    }
    if ( modrm.mod != 0x03 || (data->opcode == MUL && modrm.reg != 0x4) ||
        data->rex.lowerNibble != 0x4 || !data->rex.w ) {
        return NULL;
    }
    if ( data->opcode == ADD_REG_IMM8 && modrm.reg == 0x7) {
        sprintf(string, "cmp\t");
    }
    int firstReg = modrm.reg | (data->rex.r << 3); 
    int secondReg = modrm.rm | (data->rex.b << 3); 
    if ( data->opcode == MUL_REG_REG ) {
        int tmp = firstReg;
        firstReg = secondReg;
        secondReg = tmp;
    }
    if ( data->opcode != MUL && data->opcode != ADD_REG_IMM8 ) {
        regValue2String(firstReg, string);
        strcat(string, ",");
    }
    data->index++;
    return regValue2String(secondReg, string);
}

bool findOpcode(instructionData* data, char* string) {
    if ( data->isEscaped ) {
        return findSecondaryOpcode(data->opcode, string);
    }
    const char *primaryTable[TABLE_SIZE] = {
	[0x05] = "add",
	[0x35] = "xor",
	[0x39] = "cmp", [0x3B] = "cmp", [0x3D] = "cmp",
	[0x50] = "push", [0x51] = "push", [0x52] = "push",
    [0x53] = "push", [0x54] = "push", [0x55] = "push",
	[0x56] = "push", [0x57] = "push",
	[0x58] = "pop", [0x59] = "pop", [0x5a] = "pop",
	[0x5b] = "pop", [0x5c] = "pop", [0x5d] = "pop",
	[0x5e] = "pop", [0x5f] = "pop",
	[0x68] = "push",
	[0x72] = "jb",
	[0x74] = "je",
	[0x75] = "jne",
    [0x83] = "add",
	[0x89] = "mov", [0x8B] = "mov",
	[0x90] = "nop",
	[0xC3] = "ret",
	[0xCC] = "int3",
	[0xE8] = "call",
	[0xE9] = "jmp", [0xEB] = "jmp",
	[0xF7] = "mul"
    };
    if ( !primaryTable[data->opcode] ) {
        return false;
    }
    strcat(string, primaryTable[data->opcode]);
    if ( data->opcode == RET || data->opcode == NOP || data->opcode == INT3) {
        return true;
    }
    strcat(string, "\t");
    return true;
}


void getExpectedParams(instructionData* data) {
        if ( data->opcode == JMP_REL32OFF || data->opcode == JE_REL32OFF || data->opcode == JNE_REL32OFF ||
                data->opcode == JB_REL32OFF || data->opcode == CALL_REL32OFF ) {
                data->expectedParams[0] = DISPLACEMENT_32;
        }

        if ( data->opcode == PUSH_IMM64 || data->opcode == XOR_REG_IMM32 ||
                data->opcode == ADD_REG_IMM32 || data->opcode == CMP_REG_IMM32 ) {
                data->expectedParams[0] = IMM64;
        }


        if ( (data->opcode >= PUSH_REG64 && data->opcode <= PUSH_REG64 + 0x07)
                || (data->opcode >= POP_REG64 && data->opcode <= POP_REG64 + 0x07) ) {
                data->expectedParams[0] = RQ;
        }

        if ( data->opcode == CMP_REG_MEM || data->opcode == CMP_MEM_REG ||
                data->opcode == MUL || data->opcode == MOV_REG_MEM || 
                data->opcode == MOV_MEM_REG || data->opcode == MUL_REG_REG ||
                data->opcode == ADD_REG_IMM8 ) {
                data->expectedParams[0] = MODrm;
        }

        if ( data->opcode == JMP_REL8OFF || data->opcode == JE_REL8OFF || data->opcode == JNE_REL8OFF ||
                data->opcode == JB_REL8OFF ) {
                data->expectedParams[0] = DISPLACEMENT_8;
        }
        if ( data->opcode == ADD_REG_IMM8 ) {
                data->expectedParams[1] = IMM8;
        }

}

int searchLabelIndex(instructionData* data, uint32_t jumpTo) {
    for ( int i = 0; i < 2048; i++ ) {
        if ( data->transitions[i].to == jumpTo ) {
            return i;
        }
    }
    return -1;
}

void writeLabelIndex(instructionData* data, uint32_t jumpTo, bool fallthrough) {
    for ( int i = 0; i < 2048; i++ ) {
        if ( data->transitions[i].to == 0xffffffff ) {
            data->transitions[i].from = jumpTo < 0xffffff ? (uint32_t)data->index + data->offset : 0xffffffff;
            data->transitions[i].to = jumpTo < 0xffffff ? jumpTo : (uint32_t)data->index + data->offset;
            data->transitions[i].fallthrough = fallthrough;
            return;
        }
    }
}

#define BAD_BYTE() sprintf(result, "Unknown: 0x%x", data->instruction[data->index])

bool decodeInstruction(instructionData* data, int length, char result[40]) {
    char buffer[40] = { 0 };
    char tmp[10] = { 0 };
    if ( !findOpcode(data, result) ) {
        BAD_BYTE();
        return true;
    }
    int i = 0;
    int address;
    while( data->expectedParams[i] != NONE ) {
        if ( i >= length ) {
            return false;
        }
        switch ( data->expectedParams[i] ) {
            case DISPLACEMENT_8:
                sprintf(buffer, "%x", computeAddress(data->index, (address = get8BitValue(data))) + data->offset);
                if ( data->opcode != JMP_REL8OFF && 
                        (length > data->index || address <= 0) ) {
                    writeLabelIndex(data, data->index + data->offset, true);
                } else {
                    writeLabelIndex(data, 0xffffffff, false);
                }
                writeLabelIndex(data, computeAddress(data->index + data->offset, address), false);
                strcat(result, buffer);
                sprintf(buffer, " #<BB-0x%x>", computeAddress(data->index + data->offset, address));
                strcat(result, buffer);
                break;
            case DISPLACEMENT_32:
                if ( data->index + 3 >= length ) {
                    BAD_BYTE();
                    data->index = length + data->offset - 1;
                    return true; 
                }
                sprintf(buffer, "%x", computeAddress(data->index + data->offset, (address = get32BitValue(data))) + data->offset);
                if ( data->opcode != CALL_REL32OFF ) {
                    if ( data->opcode != JMP_REL32OFF &&
                            (length > data->index || address <= 0) ) {
                        writeLabelIndex(data, data->index + data->offset, true);
                    } else {
                        writeLabelIndex(data, 0xffffffff, false);
                    }
                    writeLabelIndex(data, computeAddress(data->index + data->offset, address), false);
                }
                strcat(result, buffer);
                if ( data->opcode != CALL_REL32OFF ) {
                    sprintf(buffer, " #<BB-0x%x>", computeAddress(data->index + data->offset, address));
                    strcat(result, buffer);
                }
                
                if ( data->opcode == CALL_REL32OFF ) {
                    sprintf(buffer, " #<0x%x>", computeAddress(data->index + data->offset, address));
                    strcat(result, buffer);
                }
                break;
            case IMM64:
                if ( data->index + 3 >= length ) {
                    BAD_BYTE();
                    data->index = length + data->offset - 1;
                    return true;
                }
                sprintf(buffer, "$0x%x,", get32BitValue(data));
                strcat(result, buffer);
                if ( data->rex.w ) {
                    strcat(result, "%rax"); 
                } else {
                    strcat(result, "%eax"); 
                }
                break;
            case RQ:
                if ( findGPR(data, buffer) ) {
                    strcat(result, buffer);
                }
                break;
            case MODrm:
                if ( !findRegisters(data, result) ) {
                    BAD_BYTE();
                }
                break;
            case IMM8:
                sprintSignedHex(get8BitValue(data), buffer);
                strcpy(tmp, result + 4);
                result[4] = '\0';
                strcat(result, "$");
                strcat(result, buffer);
                strcat(result, ",");
                strcat(result, tmp);
                break;
        }
        i++;
    }
    return true;
}


void clearInstructionData(instructionData* data) {
    data->isEscaped = false;
    data->rex.lowerNibble = 0; //setting rex to invalid prefix
}

void decodeREX(instructionData* data, const uint8_t* instruction) {
    if ( isREXprefix(instruction[data->index]) ) {
        data->rex.lowerNibble = instruction[data->index] >> 4;
        data->rex.w = instruction[data->index] >> 3;
        data->rex.r = instruction[data->index] >> 2;
        data->rex.x = instruction[data->index] >> 1;
        data->rex.b = instruction[data->index];
        data->index++;
    }
}


bool decodeSingleInstruction(int length, instructionData* data, char* strInstr) {
    decodeREX(data, data->instruction);
    if ( isEscaped(data->instruction[data->index]) ) {
        data->isEscaped = true;
        data->index++;
    }
    data->opcode = data->instruction[data->index];
    data->index++;
    params expectedParams[4] = { NONE };
    data->expectedParams = expectedParams;
    getExpectedParams(data);
    return decodeInstruction(data, length, strInstr);
}


void decodeAll(int length, const uint8_t* instruction, int offset) {
    instructionData data = {0, { 0 }, false, 0, NULL, instruction, { {0, 0, false} }, offset};
    memset(data.transitions, 0xff, 2048 * sizeof(transition));
    data.transitions[0].to = offset;
    char strInstr[2048][40] = { { 0 } };
    while ( data.index < length ) {
        int fallbackIndex = data.index;
        if ( !decodeSingleInstruction(length, &data, strInstr[data.index]) ) {
            return;
        }
        if ( !strncmp(strInstr[fallbackIndex], "Unknown", 7) ) {
            for ( int i = fallbackIndex; i < data.index; i++ ) {
                sprintf(strInstr[i], "Unknown: 0x%x", data.instruction[i]);
            }
        }
        clearInstructionData(&data);
    }
    for ( int i = 0; i < 2048; i++ ) {
        if ( !strInstr[i][0] ) {
            continue;
        }
        int labelIndex = searchLabelIndex(&data, i + offset);
        if ( labelIndex != -1 ) {
            printf("BB-0x%x:\n", i + offset);
        }
        printf("%x:\t", i + offset);
        printf("%s\n", strInstr[i]);
    }
}
