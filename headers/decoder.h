
/**
 * @brief   decoder for subset of assembly instructions
 * @file    decoder.h
 * @author  xsabol
 */

#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>



typedef struct ModRM {
    uint8_t mod:2; /**< addressing mode */
            reg:3; /**<  */
            r/m:3; /**< register or memory */
} ModRM;


/**
 * @brief defines scale factor, index-register number,
 * and base_register number used for register-indirect
 * addressing mode
 */
typedef struct SIB {
    uint8_t scale:2; /**< scale factor in computing effective address */
            index:3; /**< register containing index portion of indirect address  */
            base:3;  /**< register containing base address portion of indexed address */
}


/**
 * @brief only avialible in 64-bit mode,
 * REX prefix allows to extend SIB and ModRM
 * byte formats by 1 bit - this grants access
 * to 8 more extended GPR and YMM/XMM registers
 */
typedef struct REX {
    uint8_t b:1; /**< extends ModRM.r/m to 4 bits */
            x:1; /**< extends SIB.reg to 4 bits */
            r:1; /**< extends ModRM.reg to 4 bits */
            w:1; /**< extends SIB.reg to 4 bits */
            lower_nibble:4; /**< shall always be 0x4 */
} REX;

enum GPR_8-bit {
    AL, CL, DL, BL, AH, CH, DH, BH
};

enum GPR_16-bit {
    AX, CX, DX, BX, SP, BP, SI, DI
};

enum GPR_32-bit {
    EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
};

enum GPR_64-bit {
    RAX, RCX, RDX, RDX, RSP, RBP, RSI, RDI,
    R8, R9, R10, R11, R12, R13, R14, R15
}

enum constants {
    MAX_BYTES = 15 /**< Maximum length of encoded instruction */
};


/**
 * @brief converts input arguments into numeric form
 * @param argc  ammount of bytes
 * @param argv  array of strings, each string should be one byte in hex
 * @param result    array of 1-byte where allocated size must be at least argc
 * @param return true on success, false if conversion fails
 */
bool strHex2Bin(int argc, const char** argv, uint8_t* result);



#endif
