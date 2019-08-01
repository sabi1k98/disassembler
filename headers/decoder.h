
/**
 * @brief   decoder for subset of assembly instructions
 * @file    decoder.h
 * @author  xsabol
 */

#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


#define MAX_SIZE 2048
#define REGISTER_AMMOUNT 16
#define TABLE_SIZE 256
#define TOP_4_BYTES 0xF0

typedef struct ModRM {
    uint8_t mod:2, /**< addressing mode */
            reg:3, /**<  */
            rm:3; /**< register or memory */
} ModRM;


/**
 * @brief operand which defines scale factor, index-register number,
 * and base_register number used for register-indirect
 * addressing mode
 * Currently useless in this assignment, but might be used later
 */
typedef struct SIB {
    uint8_t scale:2, /**< scale factor in computing effective address */
            index:3, /**< register containing index portion of indirect address  */
            base:3;  /**< register containing base address portion of indexed address */
} SIB;


/**
 * @brief only avialible in 64-bit mode,
 * REX prefix allows to extend SIB and ModRM
 * byte formats by 1 bit - this grants access
 * to 8 more extended GPR and YMM/XMM registers
 */
typedef struct {
    uint8_t b:1, /**< extends ModRM.r/m to 4 bits */
            x:1, /**< extends SIB.reg to 4 bits */
            r:1, /**< extends ModRM.reg to 4 bits */
            w:1, /**< extends SIB.reg to 4 bits */
            lowerNibble:4; /**< shall always be 0x4 */
} REX;

typedef struct {
    uint32_t from;
    uint32_t to;
    bool fallthrough;
} transition;

enum GPR_64bit {
    RAX, RCX, RDX, RBX, RSP, RBP, RSI, RDI,
    R8, R9, R10, R11, R12, R13, R14, R15
};

/**
 * @brief opcodes from primary table
 */
enum primary_opcodes {
    JMP_REL8OFF = 0xEB,
    JMP_REL32OFF = 0xE9,
    JE_REL8OFF = 0X74,
    JE_REL32OFF = 0x84,
    JNE_REL8OFF = 0x75,
    JB_REL8OFF = 0x72,
    RET = 0xC3,
    CALL_REL32OFF = 0xE8,
    PUSH_IMM64 = 0X68,
    PUSH_REG64 = 0x50,
    POP_REG64 = 0x58,
    INT3 = 0xCC,
    NOP = 0x90,
    XOR_REG_IMM32 = 0x35,
    ADD_REG_IMM32 = 0x05,
    CMP_REG_IMM32 = 0x3D,
    CMP_REG_MEM = 0x3B,
    CMP_MEM_REG = 0x39,
    MUL = 0xF7,
    MOV_REG_MEM = 0x8B,
    MOV_MEM_REG = 0x89,
    SECONDARY_ESCAPE = 0x0F /**< more of a prefix than opcode, this still belongs to the primary opcode table */
};

enum secondary_opcodes {
    JNE_REL32OFF = 0x85,
    JB_REL32OFF = 0x82
};

enum operands {
    NONE,
    DISPLACEMENT_8,
    DISPLACEMENT_32, 
    MODrm, 
    S_I_B, 
    IMM64, 
    RQ /**< register encoded in opcode itself */
};

typedef int params;


/**
 *  @brief Struct containing intermeddiate results
 *  from decoding instructions
 * */
typedef struct instructionData {
    uint8_t opcode; /**< instruction opcode */
    REX rex; /**< REX prefix, only valid when first 4 bits form 0x4 */
    bool isEscaped; /**< sets to true if escape byte is found */
    int index; /**< index in decoded instruction */
    params* expectedParams; /**< array which helps us interpret following bytes */
    const uint8_t* instruction; /**< array of bytes forming the decoded instruction */
    transition transitions[2048];
    int offset;
} instructionData;


/**
 *  @brief checks if byte is REX prefix
 *  @param byte: tested byte
 *  @return true if byte is a valid REX prefix
 * */
bool isREXprefix(const uint8_t byte);


int8_t ge8BitValue(instructionData* instruction);


int32_t get32BitValue(instructionData* instruction);


bool isEscaped(const uint8_t instruction);


//finds opcode from secondary opcode table
bool findSecondaryOpcode(const uint8_t opcode, char* string);

//decodes register value into string
char* regValue2String(int regValue, char* string);


char* findGPR(instructionData* data, char* string);


char* decodeMoveInstruction(instructionData* data, ModRM modrm, char* string);


void decodeAll(int length, const uint8_t* instruction, int offset);


char* findRegisters(instructionData* data, char* string);

bool decodeSingleInstruction(int length, instructionData* data, char* strInstr);

//finds opcode in primary opcode table
bool findOpcode(instructionData* data, char* string);


void getExpectedParams(instructionData* data);


bool opcodeToString(instructionData* data, char* string);


void getExpectedParams(instructionData* data);

void clearInstructionData(instructionData* data);

int searchLabelIndex(instructionData* data, uint32_t jumpTo);

uint32_t computeAddress(uint32_t current, uint32_t relative);

void assignFromBB(instructionData* data, uint32_t toAddr, int bb);

void writeLabelIndex(instructionData* data, uint32_t jumpTo, bool fallthrough);
#endif
