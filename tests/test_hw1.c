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


void put_offset32(uint8_t* instruction, uint32_t offset) {
    *((uint32_t*)instruction) = offset;
}


TEST(1B_INSTRUCTIONS) {
    instructionData data = {0, { 0 }, false, 0, NULL, NULL,  { {0, 0, false} }, 0};
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


TEST(ARITHMETIC_AND_BITWISE_INSTRUCTIONS) {
    instructionData data = {0, { 0 }, false, 0, NULL, NULL,  { {0, 0, false} }, 0};
    char resultBuffer[30] = { 0 };
    uint8_t instruction[6] = { 0 };
    data.instruction = instruction;
    SUBTEST(XOR_EAX) {
        instruction[0] = 0x35;
        put_offset32(instruction + 1, 0x89abcdef);
        CHECK(decodeSingleInstruction(5, &data, resultBuffer) != 0);
        CHECK(data.index == 5);
        CHECK_FILE(stderr, "");
        CHECK(strcmp(resultBuffer, "xor\t$0x89abcdef,%eax") == 0);
    }
    SUBTEST(XOR_RAX) {
        instruction[0] = 0x48;
        instruction[1] = 0x35;
        put_offset32(instruction + 2, 0xfedcba98);
        CHECK(decodeSingleInstruction(6, &data, resultBuffer) != 0);
        CHECK(data.index == 6);
        CHECK_FILE(stderr, "");
        CHECK(strcmp(resultBuffer, "xor\t$0xfedcba98,%rax") == 0);
    }
    SUBTEST(ADD_EAX) {
        instruction[0] = 0x05;
        put_offset32(instruction + 1, 0x5);
        CHECK(decodeSingleInstruction(5, &data, resultBuffer) != 0);
        CHECK(data.index == 5);
        CHECK_FILE(stderr, "");
        CHECK(strcmp(resultBuffer, "add\t$0x5,%eax") == 0);
    }
    SUBTEST(ADD_RAX) {
        instruction[0] = 0x48;
        instruction[1] = 0x05;
        put_offset32(instruction + 2, 0xffffffff);
        CHECK(decodeSingleInstruction(6, &data, resultBuffer) != 0);
        CHECK(data.index == 6);
        CHECK_FILE(stderr, "");
        CHECK(strcmp(resultBuffer, "add\t$0xffffffff,%rax") == 0);
    }
    SUBTEST(MUL_RAX-RDI) {
        char* expected[] = { "mul\t%rax", "mul\t%rcx", "mul\t%rdx",
                    "mul\t%rbx", "mul\t%rsp", "mul\t%rbp", "mul\t%rsi",
                    "mul\t%rdi" };
        instruction[0] = 0x48;
        instruction[1] = 0xf7;
        instruction[2] = 0xe0;
        for ( int i = 0; i < 8; i++, instruction[2]++ ) { 
            data.index = 0;
            memset(resultBuffer, '\0', 10);
            CHECK(decodeSingleInstruction(3, &data, resultBuffer) != 0);
            CHECK(data.index == 3);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(MUL_R8-R15) {
        char* expected[] = { "mul\t%r8", "mul\t%r9", "mul\t%r10",
                    "mul\t%r11", "mul\t%r12", "mul\t%r13", "mul\t%r14",
                    "mul\t%r15" };
        instruction[0] = 0x49;
        instruction[1] = 0xf7;
        instruction[2] = 0xe0;
        for ( int i = 0; i < 8; i++, instruction[2]++ ) { 
            data.index = 0;
            memset(resultBuffer, '\0', 10);
            CHECK(decodeSingleInstruction(3, &data, resultBuffer) != 0);
            CHECK(data.index == 3);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(CMP_EAX) {
        instruction[0] = 0x3d;
        put_offset32(instruction + 1, 0x100);
        CHECK(decodeSingleInstruction(5, &data, resultBuffer) != 0);
        CHECK(data.index == 5);
        CHECK_FILE(stderr, "");
        CHECK(strcmp(resultBuffer, "cmp\t$0x100,%eax") == 0);
    }
    SUBTEST(CMP_RAX) {
        instruction[0] = 0x48;
        instruction[1] = 0x3d;
        put_offset32(instruction + 2, 0xabcd);
        CHECK(decodeSingleInstruction(6, &data, resultBuffer) != 0);
        CHECK(data.index == 6);
        CHECK_FILE(stderr, "");
        CHECK(strcmp(resultBuffer, "cmp\t$0xabcd,%rax") == 0);
    }
    SUBTEST(CMP_2REGS) {
        instruction[0] = 0x48;
        instruction[1] = 0x39;
        uint8_t modRMs[] = { 0xdd, 0xfe, 0xf8, 0xda, 0xca };
        char* expected[] = { "cmp\t%rbx,%rbp", "cmp\t%rdi,%rsi", "cmp\t%rdi,%rax",
                "cmp\t%rbx,%rdx", "cmp\t%rcx,%rdx" };
        for ( int i = 0; i < 5; i++ ) {
            data.index = 0;
            memset(resultBuffer, '\0', 10);
            instruction[2] = modRMs[i];
            CHECK(decodeSingleInstruction(3, &data, resultBuffer) != 0);
            CHECK(data.index == 3);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
}


TEST(MOVE_INSTRUCTION) {
    instructionData data = {0, { 0 }, false, 0, NULL, NULL,  { {0, 0, false} }, 0};
    char resultBuffer[30] = { 0 };
    uint8_t instruction[7] = { 0 };
    data.instruction = instruction;
    SUBTEST(REGISTER_TO_REGISTER) {
        instruction[1] = 0x89;
        uint8_t REXs[] = { 0x49, 0x48, 0x4d, 0x4c, 0x48 };
        uint8_t modRMs[] = { 0xf6, 0xfe, 0xfd, 0xd2, 0xc8 };
        char* expected[] = { "mov\t%rsi,%r14", "mov\t%rdi,%rsi", "mov\t%r15,%r13",
                "mov\t%r10,%rdx", "mov\t%rcx,%rax" };
        for ( int i = 0; i < 5; i++ ) {
            data.index = 0;
            memset(resultBuffer, '\0', 10);
            instruction[0] = REXs[i];
            instruction[2] = modRMs[i];
            CHECK(decodeSingleInstruction(3, &data, resultBuffer) != 0);
            CHECK(data.index == 3);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(REGISTER_TO_MEMORY) {
        instruction[1] = 0x89;
        uint8_t REXs[] = { 0x4c, 0x4c, 0x48, 0x48 };
        uint8_t modRMs[] = { 0x95, 0x8d, 0x15, 0x15 };
        int32_t offsets[] = { -0x418, 0x3d8, 0xbc02, -0xbc02 };
        char* expected[] = { "mov\t%r10,-0x418(%rbp) # (%rbp) + -0x418", "mov\t%r9,0x3d8(%rbp) # (%rbp) + 0x3d8",
                    "mov\t%rdx,0xbc02(%rip) # 0x7 + 0xbc02", "mov\t%rdx,-0xbc02(%rip) # 0x7 + -0xbc02"};
        for ( int i = 0; i < 4; i++ ) {
            data.index = 0;
            memset(resultBuffer, '\0', 20);
            instruction[0] = REXs[i];
            instruction[2] = modRMs[i];
            put_offset32(instruction + 3, offsets[i]);
            CHECK(decodeSingleInstruction(7, &data, resultBuffer) != 0);
            CHECK(data.index == 7);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(MEMORY_TO_REGISTER) {
        instruction[1] = 0x8b;
        uint8_t REXs[] = { 0x48, 0x48, 0x48, 0x48 };
        uint8_t modRMs[] = { 0xbd, 0xbd, 0x15, 0x15 };
        int32_t offsets[] = { 0x3e0, -0x3e0, 0x12a13, -0x12a13 };
        char* expected[] = { "mov\t0x3e0(%rbp),%rdi # (%rbp) + 0x3e0", "mov\t-0x3e0(%rbp),%rdi # (%rbp) + -0x3e0",
                    "mov\t0x12a13(%rip),%rdx # 0x7 + 0x12a13", "mov\t-0x12a13(%rip),%rdx # 0x7 + -0x12a13"};
        for ( int i = 0; i < 4; i++ ) {
            data.index = 0;
            memset(resultBuffer, '\0', 20);
            instruction[0] = REXs[i];
            instruction[2] = modRMs[i];
            put_offset32(instruction + 3, offsets[i]);
            CHECK(decodeSingleInstruction(7, &data, resultBuffer) != 0);
            CHECK(data.index == 7);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
}

TEST(JUMPS) {
    instructionData data = {0, { 0 }, false, 0, NULL, NULL,  { {0, 0, false} }, 0};
    char resultBuffer[30] = { 0 };
    uint8_t instruction[6] = { 0 };
    data.instruction = instruction;
    SUBTEST(JMP_REL8OFF) {
        instruction[0] = 0xeb;
        uint8_t offsets[] = { 0x75, 0xfe };
        char* expected[] = { "jmp\t77 #<BB-0x77>", "jmp\t0 #<BB-0x0>" };
        for ( int i = 0; i < 2; i++ ) {
            data.index = 0;
            memset(resultBuffer, '\0', 30);
            instruction[1] = offsets[i];
            CHECK(decodeSingleInstruction(2, &data, resultBuffer) != 0);
            CHECK(data.index == 2);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(JMP_REL32OFF) {
        instruction[0] = 0xe9;
        uint32_t offsets[] = { 0x12345678, 0xffffffff };
        char* expected[] = { "jmp\t1234567d #<BB-0x1234567d>", "jmp\t4 #<BB-0x4>" };
        for ( int i = 0; i < 2; i++ ) {
            data.index = 0;
            memset(resultBuffer, '\0', 30);
            put_offset32(instruction + 1, offsets[i]);
            CHECK(decodeSingleInstruction(5, &data, resultBuffer) != 0);
            CHECK(data.index == 5);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(JE_REL8OFF) {
        instruction[0] = 0x74;
        uint8_t offsets[] = { 0x75, 0xfe };
        char* expected[] = { "je\t77 #<BB-0x77>", "je\t0 #<BB-0x0>" };
        for ( int i = 0; i < 2; i++ ) {
            data.index = 0;
            memset(resultBuffer, '\0', 30);
            instruction[1] = offsets[i];
            CHECK(decodeSingleInstruction(2, &data, resultBuffer) != 0);
            CHECK(data.index == 2);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(JE_REL32OFF) {
        instruction[0] = 0x0f;
        instruction[1] = 0x84;
        uint32_t offsets[] = { 0x12345678, 0xffffffff };
        char* expected[] = { "je\t1234567e #<BB-0x1234567e>", "je\t5 #<BB-0x5>" };
        for ( int i = 0; i < 2; i++ ) {
            data.index = 0;
            memset(resultBuffer, '\0', 30);
            put_offset32(instruction + 2, offsets[i]);
            CHECK(decodeSingleInstruction(6, &data, resultBuffer) != 0);
            CHECK(data.index == 6);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(JNE_REL8OFF) {
        instruction[0] = 0x75;
        uint8_t offsets[] = { 0x75, 0xfe };
        char* expected[] = { "jne\t77 #<BB-0x77>", "jne\t0 #<BB-0x0>" };
        for ( int i = 0; i < 2; i++ ) {
            data.index = 0;
            memset(resultBuffer, '\0', 30);
            instruction[1] = offsets[i];
            CHECK(decodeSingleInstruction(2, &data, resultBuffer) != 0);
            CHECK(data.index == 2);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(JNE_REL32OFF) {
        instruction[0] = 0x0f;
        instruction[1] = 0x85;
        uint32_t offsets[] = { 0x12345678, 0xffffffff };
        char* expected[] = { "jne\t1234567e #<BB-0x1234567e>", "jne\t5 #<BB-0x5>" };
        for ( int i = 0; i < 2; i++ ) {
            data.index = 0;
            memset(resultBuffer, '\0', 30);
            put_offset32(instruction + 2, offsets[i]);
            CHECK(decodeSingleInstruction(6, &data, resultBuffer) != 0);
            CHECK(data.index == 6);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(JB_REL8OFF) {
        instruction[0] = 0x72;
        uint8_t offsets[] = { 0x75, 0xfe };
        char* expected[] = { "jb\t77 #<BB-0x77>", "jb\t0 #<BB-0x0>" };
        for ( int i = 0; i < 2; i++ ) {
            data.index = 0;
            memset(resultBuffer, '\0', 30);
            instruction[1] = offsets[i];
            CHECK(decodeSingleInstruction(2, &data, resultBuffer) != 0);
            CHECK(data.index == 2);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
    SUBTEST(JB_REL32OFF) {
        instruction[0] = 0x0f;
        instruction[1] = 0x82;
        uint32_t offsets[] = { 0x12345678, 0xffffffff };
        char* expected[] = { "jb\t1234567e #<BB-0x1234567e>", "jb\t5 #<BB-0x5>" };
        for ( int i = 0; i < 2; i++ ) {
            data.index = 0;
            memset(resultBuffer, '\0', 30);
            put_offset32(instruction + 2, offsets[i]);
            CHECK(decodeSingleInstruction(6, &data, resultBuffer) != 0);
            CHECK(data.index == 6);
            CHECK_FILE(stderr, "");
            CHECK(strcmp(resultBuffer, expected[i]) == 0);
        }
    }
}
