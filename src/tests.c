#define _POSIX_C_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define CUT
#ifndef DEBUG
#  define CUT_FORK_MODE
#endif
#define CUT_MAIN
#include "decoder.h"
#include "cfg.h"
#include "cut.h"
/* Testing framework cloned from:
 *  https://github.com/spito/testing.git
 */



TEST(HW01_1B_INSTRUCTIONS) {
    instructionData data = {0, { 0 }, false, 0, NULL, NULL,  { {0, 0} }};
    char resultBuffer[30] = { 0 };
    uint8_t instruction[2] = { 0 };
    data.instruction = instruction;
    SUBTEST(PUSH) {
        char* expected[] = { "push\t%rax", "push\t%rcx", "push\t%rdx",
                    "push\t%rbx", "push\t%rsp", "push\t%rbp", "push\t%rsi",
                    "push\t%rdi" };
        instruction[0] = 0x50;
        for ( int i = 0; i < 8; i++, instruction[0]++ ) { 
            data.index = 0;
            memset(resultBuffer, '\0', 10);
            CHECK(decodeSingleInstruction(1, &data, resultBuffer) != 0);
            CHECK(data.index == 1);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(PUSH_WITH_REX) {
        char* expected[10] = { "push\t%r8", "push\t%r9", "push\t%r10",
                    "push\t%r11", "push\t%r12", "push\t%r13", "push\t%r14",
                    "push\t%r15" };
        instruction[0] = 0x41;
        instruction[1] = 0x50;
        for ( int i = 0; i < 8; i++, instruction[1]++ ) { 
            data.index = 0;
            memset(resultBuffer, '\0', 10);
            CHECK(decodeSingleInstruction(2, &data, resultBuffer) != 0);
            CHECK(data.index == 2);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }

    }
    SUBTEST(POP) {
        char* expected[] = { "pop\t%rax", "pop\t%rcx", "pop\t%rdx",
                        "pop\t%rbx", "pop\t%rsp", "pop\t%rbp", "pop\t%rsi",
                    "pop\t%rdi" };
        instruction[0] = 0x58;
        for ( int i = 0; i < 8; i++, instruction[0]++ ) { 
            data.index = 0;
            memset(resultBuffer, '\0', 10);
            CHECK(decodeSingleInstruction(1, &data, resultBuffer) != 0);
            CHECK(data.index == 1);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }

    SUBTEST(POP_WITH_REX) {
        char* expected[10] = { "pop\t%r8", "pop\t%r9", "pop\t%r10",
                    "pop\t%r11", "pop\t%r12", "pop\t%r13", "pop\t%r14",
                    "pop\t%r15" };
        instruction[0] = 0x41;
        instruction[1] = 0x58;
        for ( int i = 0; i < 8; i++, instruction[1]++ ) { 
            data.index = 0;
            memset(resultBuffer, '\0', 10);
            CHECK(decodeSingleInstruction(2, &data, resultBuffer) != 0);
            CHECK(data.index == 2);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(RET) {
        instruction[0] = 0xc3;
        CHECK(decodeSingleInstruction(1, &data, resultBuffer) != 0);
        CHECK(data.index == 1);
        CHECK_FILE(stderr, "");
        CHECK(strcmp(resultBuffer, "ret") == 0);
    }
    SUBTEST(NOP) {
        instruction[0] = 0x90;
        CHECK(decodeSingleInstruction(1, &data, resultBuffer) != 0);
        CHECK(data.index == 1);
        CHECK_FILE(stderr, "");
        CHECK(strcmp(resultBuffer, "nop") == 0);
    }
    SUBTEST(INT3) {
        instruction[0] = 0xcc;
        CHECK(decodeSingleInstruction(1, &data, resultBuffer) != 0);
        CHECK(data.index == 1);
        CHECK_FILE(stderr, "");
        CHECK(strcmp(resultBuffer, "int3") == 0);
    }
}
