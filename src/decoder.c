#include "decoder.h"


bool isREXprefix(const uint8_t byte) {
    return (byte & TOP_4_BYTES) == 0x40;
}

bool isEscaped(const uint8_t instruction) {
    return instruction == SECONDARY_ESCAPE;
}


int8_t get8BitValue(instructionData* data) {
    return data->instruction[data->index];
}


int32_t get32BitValue(instructionData* data) {
    data->instruction += data->index;
    uint32_t ret = *((uint32_t*) data->instruction);
    data->instruction -= data->index;
    return ret;
}

uint32_t computeAddress(uint32_t current, uint32_t relative) {
    return current + (int32_t)relative + 1;
}


bool findSecondaryOpcode(const uint8_t opcode, char* string) {
    if ( opcode == JNE_REL32OFF ) {
        sprintf(string, "%s\t", "jne");
        return true; 
    }
    if ( opcode == JB_REL32OFF ) { 
        sprintf(string, "%s\t", "jb");
        return true; 
    }
    if ( opcode == JE_REL32OFF ) { 
        sprintf(string, "%s\t", "je");
        return true; 
    }
    return false;
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


char* decodeMoveInstruction(instructionData* data, ModRM modrm, char* string) { 
    if ( modrm.rm != 0x5) { //ONLY 0b101 as r/m can encode RIP-relative addressing
        return NULL;
    }
    int regNum = modrm.reg | (data->rex.r << 3); 
    char source[20] = { 0 };
    char destination[10] = { 0 };
    data->index++;
    if (modrm.mod == 0x00) { //RIP-relative addressing
        sprintf(source, "0x%x(%%rip)", get32BitValue(data));
    } else { //base + offset with RBP as base
        sprintf(source, "0x%x(%%rbp)", get8BitValue(data));
    }
    if ( data->opcode == MOV_REG_MEM ) {
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
    const char *primaryTable[TABLE_SIZE] = {
	[0x05] = "add",
	[0x35] = "xor",
	[0x39] = "cmp", [0x3B] = "cmp", [0x3D] = "cmp",
	[0x50] = "push", [0x51] = "push", [0x52] = "push",
    [0x53] = "push", [0x54] = "push", [0x55] = "push",
	[0x56] = "push", [0x57] = "push",
	[0x58] = "pop", [0x59] = "pop", [0x60] = "pop",
	[0x61] = "pop", [0x62] = "pop", [0x63] = "pop",
	[0x64] = "pop", [0x65] = "pop",
	[0x68] = "push",
	[0x72] = "jb",
	[0x74] = "je",
	[0x75] = "jne",
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
    if ( data->opcode == XOR_REG_IMM32 || data->opcode == CMP_REG_IMM32
            || data->opcode == ADD_REG_IMM32 ) {
        if ( data->rex.w ) {
            strcat(string, "\t%rax, "); 
        } else {
            strcat(string, "\t%eax, "); 
        }
    }
    strcat(string, "\t");
    return true;
}


void getExpectedParams(instructionData* data) {
        if ( data->opcode == JMP_REL8OFF || data->opcode == JE_REL8OFF || data->opcode == JNE_REL8OFF ||
                data->opcode == JB_REL8OFF ) {
                data->expectedParams[0] = DISPLACEMENT_8;
        }

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
                data->opcode == MOV_MEM_REG) {
                data->expectedParams[0] = MODrm;
        }

}


void assignFromBB(instructionData* data, uint32_t toAddr, int bb) {
    for ( int i = 0; data->basicBlocks[i].to < 0xffffff; i++ ) {
        if ( toAddr == data->basicBlocks[i].to ) {
            data->basicBlocks[i].toBB = bb;
        }
    }
}

int searchLabelIndex(instructionData* data, uint32_t jumpTo) {
    for ( int i = 0; i < 2048; i++ ) {
        if ( data->basicBlocks[i].to == jumpTo ) {
            return i;
        }
    }
    return -1;
}

void writeLabelIndex(instructionData* data, uint32_t jumpTo) {
    for ( int i = 0; i < 2048; i++ ) {
        if ( data->basicBlocks[i].to == 0xffffffff ) {
            data->basicBlocks[i].from = data->index;
            data->basicBlocks[i].to = jumpTo;
            data->basicBlocks[i].fromBB = data->currentBB;
            return;
        }
    }
}


bool decodeInstruction(instructionData* data, int length, char result[20]) {
    char buffer[20] = { 0 };
    if ( !findOpcode(data, result) ) {
        fprintf(stderr, "Unknown instruction\n");
        return false;
    }
    int i = 0;
    int address;
    while( data->expectedParams[i] != NONE ) {
        if ( i == length ) {
            printf("Unknown instruction\n");
            return false;
        }
        switch ( data->expectedParams[i] ) {
            case DISPLACEMENT_8:
                sprintf(buffer, "%x", computeAddress(data->index, (address = get8BitValue(data))));
                writeLabelIndex(data, data->index + 1);
                writeLabelIndex(data, computeAddress(data->index, address));
                strcat(result, buffer);
                sprintf(buffer, "  #<BB-0x%x>", computeAddress(data->index, address));
                strcat(result, buffer);
                data->index++;
                break;
            case DISPLACEMENT_32:
                if ( data->index + 3 > length ) {
                    printf("Unknown instruction\n");
                    return false; 
                }
                sprintf(buffer, "%x", computeAddress(data->index, (address = get32BitValue(data))));
                writeLabelIndex(data, data->index + 1);
                writeLabelIndex(data, computeAddress(data->index, address));
                strcat(result, buffer);
                sprintf(buffer, "  #<BB-0x%x>", computeAddress(data->index, address));
                strcat(result, buffer);
                data->index += sizeof(uint32_t);
                break;
            case IMM64:
                if ( data->index + 3 > length ) {
                    printf("Unknown instruction\n");
                    return false;
                }
                sprintf(buffer, "$0x%x", get32BitValue(data));
                data->index += sizeof(uint32_t);
                strcat(result, buffer);
                break;
            case RQ:
                if ( findGPR(data, buffer) ) {
                    strcat(result, buffer);
                }
                break;
            case MODrm:
                if ( !findRegisters(data, result) ) {
                    printf("Unknown instruction\n");
                    return false;
                }
                data->index++;
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

void decodeREX(instructionData* data, uint8_t* instruction) {
    if ( isREXprefix(instruction[data->index]) ) {
        data->rex.lowerNibble = instruction[data->index] >> 4;
        data->rex.w = instruction[data->index] >> 3;
        data->rex.r = instruction[data->index] >> 2;
        data->rex.x = instruction[data->index] >> 1;
        data->rex.b = instruction[data->index];
        data->index++;
    }
}


bool decodeSingleInstruction(int length, uint8_t* instruction, instructionData* data, char* strInstr) {
    memset(strInstr, '\0', 20);
    printf("%x:\t", data->index);
    
    if ( isEscaped(instruction[data->index]) ) {
        data->isEscaped = true;
        data->index++;
    }
    decodeREX(data, instruction);
    data->opcode = instruction[data->index];
    data->index++;
    params expectedParams[4] = { NONE };
    data->expectedParams = expectedParams;
    getExpectedParams(data);
    return decodeInstruction(data, length, strInstr);
}


void decodeAll(int length, uint8_t* instruction) {
    instructionData data = {0, { 0 }, false, 0, NULL, instruction, { {0, 0, 0, 0} }, 0};
    memset(data.basicBlocks, 0xff, 2048 * sizeof(transition));
    data.basicBlocks[0].to = 0x0;
    char strInstr[20];
    while ( data.index < length ) {
        int labelIndex = searchLabelIndex(&data, data.index);
        if ( labelIndex != -1 ) {
            printf("BB-0x%x:\n", data.index);
            data.currentBB++;
            assignFromBB(&data, data.index, data.currentBB);
        }
        if ( !decodeSingleInstruction(length, instruction, &data, strInstr) ) {
            return;
        }
        printf("%s\n", strInstr);
        clearInstructionData(&data);
    }
}
