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

const uint8_t JUMP_CFLW[] = { 0xe9, 0x0b, 0x00, 0x00, 0x00,
                        0x48, 0x05, 0x42, 0x42, 0x42,
                        0x42, 0xe9, 0x05, 0x00, 0x00,
                        0x00, 0xe9, 0xf0, 0xff, 0xff,
                        0xff, 0x55 };

const uint8_t NOPS[] = { 0x90, 0x74, 0x03, 0x90, 0x90,
                          0x90, 0x90, 0x74, 0xf7, 0x90 };

const uint8_t LOOP[] = { 0x48, 0x05, 0x42, 0x42, 0x42,
                         0x42, 0x90, 0x90, 0x90, 0x90,
                         0x90, 0x90, 0xe9, 0xf5, 0xff,
                         0xff, 0xff };



TEST(MULTIPLE_INSTRUCTIONS_WITH_CONTROL_FLOW) {
    SUBTEST(NOPS) {
        const char* expected = "BB-0x0:\n"
                               "0:\tnop\n"
                               "1:\tje\t6 #<BB-0x6>\n"
                               "BB-0x3:\n"
                               "3:\tnop\n"
                               "4:\tnop\n"
                               "5:\tnop\n"
                               "BB-0x6:\n"
                               "6:\tnop\n"
                               "7:\tje\t0 #<BB-0x0>\n"
                               "BB-0x9:\n"
                               "9:\tnop\n";
        decodeAll(10, NOPS);
        CHECK_FILE(stdout, expected);

    }
    SUBTEST(JMPs_NO_BRANCHING_LOTS_OF_BBS) {
        const char* expected = "BB-0x0:\n"
                                "0:\tjmp\t10 #<BB-0x10>\n"
                                "BB-0x5:\n"
                                "5:\tadd\t$0x42424242,%rax\n"
                                "b:\tjmp\t15 #<BB-0x15>\n"
                                "BB-0x10:\n"
                                "10:\tjmp\t5 #<BB-0x5>\n"
                                "BB-0x15:\n"
                                "15:\tpush\t%rbp\n";
        decodeAll(22, JUMP_CFLW);
        CHECK_FILE(stdout, expected);
    }
    SUBTEST(BIG_LOOP) {
        const char* expected = "BB-0x0:\n"
                                "0:\tadd\t$0x42424242,%rax\n"
                                "BB-0x6:\n"
                                "6:\tnop\n"
                                "7:\tnop\n"
                                "8:\tnop\n"
                                "9:\tnop\n"
                                "a:\tnop\n"
                                "b:\tnop\n"
                                "c:\tjmp\t6 #<BB-0x6>\n";
        decodeAll(17, LOOP);
        CHECK_FILE(stdout, expected);
        CHECK_FILE(stderr, "");
    }
}



TEST(CFG_CREATION) {
    SUBTEST(NOPS) {
        const char* expected = "digraph G {\n"
                "0 [ shape=rectangle label=\"BB-0x0:\\l0:\tnop\\l1:\tje\t6 #<BB-0x6>\\l\" ]\n"
                "3 [ shape=rectangle label=\"BB-0x3:\\l3:\tnop\\l4:\tnop\\l5:\tnop\\l\" ]\n"
                "6 [ shape=rectangle label=\"BB-0x6:\\l6:\tnop\\l7:\tje\t0 #<BB-0x0>\\l\" ]\n"
                "9 [ shape=rectangle label=\"BB-0x9:\\l9:\tnop\\l\" ]\n"
                "0 -> 3\n"
                "0 -> 6\n"
                "6 -> 9\n"
                "6 -> 0\n"
                "3 -> 6\n"
                "}";
        makeGraph(10, NOPS);
        CHECK_FILE(stdout, expected);
        CHECK_FILE(stderr, "");

    }
    SUBTEST(JMPs_NO_BRANCHING_LOTS_OF_BBS) {
        const char* expected =  "digraph G {\n"
                                "0 [ shape=rectangle label=\"BB-0x0:\\l0:\tjmp\t10 #<BB-0x10>\\l\" ]\n"
                                "5 [ shape=rectangle label=\"BB-0x5:\\l5:\tadd\t$0x42424242,%rax\\lb:	jmp	15 #<BB-0x15>\\l\" ]\n"
                                "16 [ shape=rectangle label=\"BB-0x10:\\l10:\tjmp\t5 #<BB-0x5>\\l\" ]\n"
                                "21 [ shape=rectangle label=\"BB-0x15:\\l15:\tpush\t%rbp\\l\" ]\n"
                                "0 -> 16\n"
                                "5 -> 21\n"
                                "16 -> 5\n"
                                "}"; 
        makeGraph(22, JUMP_CFLW);
        CHECK_FILE(stdout, expected);
        CHECK_FILE(stderr, "");
    }
    SUBTEST(BIG_LOOP) {
        const char* expected = "digraph G {\n"
                               "0 [ shape=rectangle label=\"BB-0x0:\\l0:\tadd\t$0x42424242,%rax\\l\" ]\n"
                               "6 [ shape=rectangle label=\"BB-0x6:\\l6:\tnop\\l7:\tnop\\l8:\tnop\\l9:\tnop\\la:\tnop\\lb:\tnop\\lc:\tjmp\t6 #<BB-0x6>\\l\" ]\n"
                               "6 -> 6\n"
                               "0 -> 6\n"
                               "}";

        makeGraph(17, LOOP);
        CHECK_FILE(stdout, expected);
        CHECK_FILE(stderr, "");
    }
}

