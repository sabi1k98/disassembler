
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



typedef struct ModRM {
    uint8_t mod:2, /**< addressing mode */
            reg:3, /**<  */
            rm:3; /**< register or memory */
} ModRM;


/**
 * @brief defines scale factor, index-register number,
 * and base_register number used for register-indirect
 * addressing mode
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
typedef struct REX {
    uint8_t b:1, /**< extends ModRM.r/m to 4 bits */
            x:1, /**< extends SIB.reg to 4 bits */
            r:1, /**< extends ModRM.reg to 4 bits */
            w:1, /**< extends SIB.reg to 4 bits */
            lowerNibble:4; /**< shall always be 0x4 */
} REX;

enum GPR_8bit {
    AL, CL, DL, BL, AH, CH, DH, BH
};

enum GPR_16bit {
    AX, CX, DX, BX, SP, BP, SI, DI
};

enum GPR_32bit {
    EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
};

enum GPR_64bit {
    RAX, RCX, RDX, RBX, RSP, RBP, RSI, RDI,
    R8, R9, R10, R11, R12, R13, R14, R15
};

enum primary_opcodes {
    JMP_REL8OFF = 0xEB,
    JMP_REL32OFF = 0xE9,
    JE_REL8OFF = 0X74,
    JE_REL32OFF = 0x84,
    JNE_REL8OFF = 0X75,
    JB_REL8OFF = 0x72,
    RET = 0XC3,
    CALL_REL32OFF = 0XE8,
    PUSH_IMM64 = 0X68,
    PUSH_REG64 = 0x50,
    POP_REG64 = 0x58,
    INT3 = 0xCC,
    NOP = 0x90,
    XOR_REG_IMM32 = 0X35,
    ADD_REG_IMM32 = 0X05,
    CMP_REG_IMM32 = 0x3D,
    CMP_REG_MEM = 0x3B,
    CMP_MEM_REG = 0x39,
    SECONDARY_ESCAPE = 0x0F /**< more of a prefix than opcode, this still belongs to the primary opcode table */
};

enum secondary_opcodes {
    JNE_REL32OFF = 0x85,
    JB_REL32OFF = 0x82
};

enum operands {
    NONE, DISPLACEMENT_8, DISPLACEMENT_32, MODrm, S_I_B, IMM64, RQ, ERROR
};

typedef int params;


bool getExpectedParams(uint8_t opcode, int remaining, params* params);

enum constants {
    MAX_BYTES = 15 /**< Maximum length of encoded instruction */
};


typedef struct instructionData {
    uint8_t opcode;
    REX rex;
    bool isEscaped;
    int index;
    params* expectedParams;
} instructionData;



bool isREXprefix(const uint8_t byte);

uint8_t ge8BitDisplacement(const uint8_t* instruction, int start);

uint32_t get32BitDisplacement(const uint8_t* instruction, int start);

void decode(int length, uint8_t* instruction);

bool opcodeToString(instructionData* data, char* string);

bool decodeOpcode(uint8_t opcode);

REX storeREX(uint8_t prefix);

#endif
